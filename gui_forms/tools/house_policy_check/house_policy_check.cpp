#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprConcepts.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/QualTypeNames.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

namespace {

llvm::cl::OptionCategory policy_category("GUI.Forms house-policy options");

llvm::cl::opt<std::string> source_root(
    "source-root", llvm::cl::Required,
    llvm::cl::desc("Absolute path to the gui_forms source directory"),
    llvm::cl::cat(policy_category));

llvm::cl::opt<std::string> json_output(
    "json-output", llvm::cl::Required,
    llvm::cl::desc("Path for the stable JSON finding ledger"),
    llvm::cl::cat(policy_category));

llvm::cl::opt<std::string> policy_mode(
    "mode", llvm::cl::init("inventory"),
    llvm::cl::desc("Policy mode: inventory or closure"),
    llvm::cl::cat(policy_category));

llvm::cl::opt<std::string> source_scope(
    "source-scope", llvm::cl::init("production"),
    llvm::cl::desc("Source scope: production or first-party"),
    llvm::cl::cat(policy_category));

llvm::cl::opt<bool> quiet(
    "quiet", llvm::cl::init(false),
    llvm::cl::desc("Suppress individual text diagnostics"),
    llvm::cl::cat(policy_category));

llvm::cl::opt<std::string> pointer_rewrite_output(
    "pointer-rewrite-output", llvm::cl::init(""),
    llvm::cl::desc("Optional path for verified pointer-arrow rewrite offsets"),
    llvm::cl::cat(policy_category));

llvm::cl::opt<std::string> auto_rewrite_output(
    "auto-rewrite-output", llvm::cl::init(""),
    llvm::cl::desc("Optional path for resolved auto-type rewrite candidates"),
    llvm::cl::cat(policy_category));

struct Finding final {
    std::string path;
    unsigned line{};
    unsigned column{};
    std::string enclosing_symbol;
    std::string construct_kind;
    std::string replacement_category;
    std::string disposition;
    std::string exception_identifier;
};

struct PointerArrowRewrite final {
    std::string path;
    std::uint64_t file_size{};
    std::uint64_t base_offset{};
    std::uint64_t operator_offset{};
};

struct AutoTypeRewrite final {
    std::string path;
    std::uint64_t file_size{};
    std::uint64_t token_offset{};
    std::string replacement;
    bool safe{};
    std::string reason;
};

[[nodiscard]] bool finding_less(const Finding& left,
                                const Finding& right) noexcept {
    if (left.path != right.path) return left.path < right.path;
    if (left.line != right.line) return left.line < right.line;
    if (left.column != right.column) return left.column < right.column;
    return left.construct_kind < right.construct_kind;
}

[[nodiscard]] bool pointer_rewrite_less(
    const PointerArrowRewrite& left,
    const PointerArrowRewrite& right) noexcept {
    if (left.path != right.path) return left.path < right.path;
    if (left.base_offset != right.base_offset) {
        return left.base_offset < right.base_offset;
    }
    return left.operator_offset < right.operator_offset;
}

[[nodiscard]] bool auto_type_rewrite_less(
    const AutoTypeRewrite& left,
    const AutoTypeRewrite& right) noexcept {
    if (left.path != right.path) return left.path < right.path;
    return left.token_offset < right.token_offset;
}

[[nodiscard]] std::string normalized_path(llvm::StringRef input) {
    llvm::SmallString<512U> path(input);
    llvm::sys::path::remove_dots(path, true);
    return std::string(path.str());
}

[[nodiscard]] bool contains_component(llvm::StringRef path,
                                      llvm::StringRef component) noexcept {
    return path.contains(component);
}

class FindingCollector final {
public:
    FindingCollector(clang::ASTContext& context, std::vector<Finding>& findings,
                     std::set<std::string>& keys,
                     std::vector<PointerArrowRewrite>& pointer_rewrites,
                     std::set<std::string>& pointer_rewrite_keys,
                     std::vector<AutoTypeRewrite>& auto_rewrites,
                     std::map<std::string, std::size_t>& auto_rewrite_indices)
        : context_(context), source_manager_(context.getSourceManager()),
          findings_(findings), keys_(keys),
          pointer_rewrites_(pointer_rewrites),
          pointer_rewrite_keys_(pointer_rewrite_keys),
          auto_rewrites_(auto_rewrites),
          auto_rewrite_indices_(auto_rewrite_indices),
          normalized_root_(normalized_path(source_root)) {}

    template <typename Node>
    void add(llvm::StringRef kind, clang::SourceLocation location,
             const Node& node, llvm::StringRef replacement,
             llvm::StringRef disposition = "violation",
             llvm::StringRef exception_identifier = "") {
        const clang::SourceLocation spelling =
            source_manager_.getSpellingLoc(location);
        if (!spelling.isValid() || source_manager_.isInSystemHeader(spelling)) {
            return;
        }
        const std::string absolute_path =
            normalized_path(source_manager_.getFilename(spelling));
        if (!is_normative_path(absolute_path)) return;
        const llvm::StringRef relative = llvm::StringRef(absolute_path).drop_front(
            normalized_root_.size() + 1U);
        const std::string path(relative);

        const unsigned line = source_manager_.getSpellingLineNumber(spelling);
        const unsigned column = source_manager_.getSpellingColumnNumber(spelling);
        const std::string enclosing = enclosing_symbol(
            clang::DynTypedNode::create(node));
        std::string effective_disposition(disposition);
        std::string effective_exception(exception_identifier);
        if (kind == "std_any" && is_tag_exception(enclosing)) {
            effective_disposition = "approved_exception";
            effective_exception = "O-011-tag";
        }

        const std::string key = path + ":" + std::to_string(line) + ":" +
            std::to_string(column) + ":" + std::string(kind);
        if (!keys_.insert(key).second) return;
        findings_.push_back(Finding{path, line, column, enclosing,
                                    std::string(kind), std::string(replacement),
                                    effective_disposition, effective_exception});
    }

    template <typename MemberExpression>
    void add_pointer_arrow(const MemberExpression& expression) {
        add("pointer_member_arrow", expression.getOperatorLoc(), expression,
            "explicit_dereference_member_access");
        if (pointer_rewrite_output.empty()) return;
        if (expression.isImplicitAccess()) return;
        if ((*(*expression.getBase()).getType()).isObjCObjectPointerType()) return;

        const clang::SourceLocation base_location =
            source_manager_.getSpellingLoc((*expression.getBase()).getBeginLoc());
        const clang::SourceLocation operator_location =
            source_manager_.getSpellingLoc(expression.getOperatorLoc());
        if (!base_location.isValid() || !operator_location.isValid() ||
            (*expression.getBase()).getBeginLoc().isMacroID() ||
            expression.getOperatorLoc().isMacroID()) {
            return;
        }
        const clang::FileID base_file = source_manager_.getFileID(base_location);
        const clang::FileID operator_file =
            source_manager_.getFileID(operator_location);
        if (base_file != operator_file) return;

        const std::string absolute_path =
            normalized_path(source_manager_.getFilename(operator_location));
        if (!is_normative_path(absolute_path)) return;
        const llvm::StringRef operator_text = clang::Lexer::getSourceText(
            clang::CharSourceRange::getTokenRange(operator_location,
                                                   operator_location),
            source_manager_, context_.getLangOpts());
        if (operator_text != "->") return;

        const std::uint64_t base_offset =
            source_manager_.getFileOffset(base_location);
        const std::uint64_t operator_offset =
            source_manager_.getFileOffset(operator_location);
        if (base_offset >= operator_offset) return;
        const llvm::StringRef relative = llvm::StringRef(absolute_path).drop_front(
            normalized_root_.size() + 1U);
        const std::string path(relative);
        const std::string key = path + ":" + std::to_string(base_offset) + ":" +
            std::to_string(operator_offset);
        if (!pointer_rewrite_keys_.insert(key).second) return;
        const std::uint64_t file_size =
            source_manager_.getBufferData(base_file).size();
        pointer_rewrites_.push_back(PointerArrowRewrite{
            path, file_size, base_offset, operator_offset});
    }

    void add_auto_type(
        clang::AutoTypeLoc location, clang::QualType semantic_type = {},
        clang::SourceLocation declarator_location = {}) {
        const clang::AutoType* type = location.getTypePtr();
        const llvm::StringRef kind = (*type).isDecltypeAuto()
            ? llvm::StringRef("decltype_auto") : llvm::StringRef("auto_type");
        add(kind, location.getBeginLoc(), location,
            "spell_explicit_named_type");
        if (auto_rewrite_output.empty() || (*type).isDecltypeAuto()) return;

        const clang::SourceLocation spelling =
            source_manager_.getSpellingLoc(location.getBeginLoc());
        if (!is_normative_location(spelling) ||
            location.getBeginLoc().isMacroID()) {
            return;
        }
        const llvm::StringRef token_text = clang::Lexer::getSourceText(
            clang::CharSourceRange::getTokenRange(spelling, spelling),
            source_manager_, context_.getLangOpts());
        if (token_text != "auto") return;

        clang::QualType deduced = semantic_type.isNull()
            ? (*type).getDeducedType() : semantic_type;
        while (!deduced.isNull() && (*deduced).isReferenceType()) {
            deduced = (*deduced).getPointeeType();
        }
        const clang::SourceLocation declarator_spelling =
            source_manager_.getSpellingLoc(declarator_location);
        if (!semantic_type.isNull() && declarator_spelling.isValid() &&
            source_manager_.getFileID(declarator_spelling) ==
                source_manager_.getFileID(spelling)) {
            const std::uint64_t declarator_offset =
                source_manager_.getFileOffset(declarator_spelling);
            const std::uint64_t auto_offset =
                source_manager_.getFileOffset(spelling);
            if (declarator_offset > auto_offset + 4U) {
                const llvm::StringRef buffer = source_manager_.getBufferData(
                    source_manager_.getFileID(spelling));
                const llvm::StringRef declarator_prefix = buffer.slice(
                    auto_offset + 4U, declarator_offset);
                for (const char character : declarator_prefix) {
                    if (character != '*') continue;
                    if (deduced.isNull() || !(*deduced).isPointerType()) {
                        deduced = clang::QualType();
                        break;
                    }
                    deduced = (*deduced).getPointeeType();
                }
            }
        }
        if (!deduced.isNull()) {
            deduced = deduced.getLocalUnqualifiedType();
        }
        if (deduced.isNull() && semantic_type.isNull()) {
            return;
        }
        std::string replacement;
        bool safe = false;
        std::string reason;
        if (deduced.isNull() || (*deduced).isDependentType()) {
            reason = "dependent_or_undeduced";
        } else {
            clang::PrintingPolicy policy(context_.getPrintingPolicy());
            policy.SuppressTagKeyword = true;
            replacement = clang::TypeName::getFullyQualifiedName(
                deduced, context_, policy, false);
            static constexpr llvm::StringLiteral standard_integer_aliases[] = {
                "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t",
                "uint32_t", "int64_t", "uint64_t", "int_fast8_t",
                "uint_fast8_t", "int_fast16_t", "uint_fast16_t",
                "int_fast32_t", "uint_fast32_t", "int_fast64_t",
                "uint_fast64_t", "int_least8_t", "uint_least8_t",
                "int_least16_t", "uint_least16_t", "int_least32_t",
                "uint_least32_t", "int_least64_t", "uint_least64_t",
                "intmax_t", "uintmax_t", "intptr_t", "uintptr_t", "size_t",
                "ptrdiff_t"};
            for (const llvm::StringRef alias : standard_integer_aliases) {
                if (replacement == alias) {
                    replacement = "std::" + replacement;
                    break;
                }
            }
            static constexpr llvm::StringLiteral standard_template_prefixes[] = {
                "shared_ptr<", "unique_ptr<", "span<"};
            for (const llvm::StringRef prefix : standard_template_prefixes) {
                if (llvm::StringRef(replacement).starts_with(prefix)) {
                    replacement = "std::" + replacement;
                    break;
                }
            }
            if (replacement == "from_chars_result" ||
                replacement == "to_chars_result") {
                replacement = "std::" + replacement;
            }
            static constexpr llvm::StringLiteral ambiguous_nested_aliases[] = {
                "iterator", "const_iterator", "reverse_iterator",
                "const_reverse_iterator", "size_type", "value_type",
                "difference_type", "reference", "const_reference",
                "pointer", "const_pointer", "time_point", "duration",
                "rep", "period", "pos_type", "off_type", "path"};
            bool ambiguous_nested_alias = false;
            for (const llvm::StringRef alias : ambiguous_nested_aliases) {
                if (replacement == alias) {
                    ambiguous_nested_alias = true;
                    break;
                }
            }
            if (llvm::StringRef(replacement).contains("element_type") ||
                llvm::StringRef(replacement).contains("dynamic_extent") ||
                llvm::StringRef(replacement).contains("common_type")) {
                ambiguous_nested_alias = true;
            }
            static constexpr llvm::StringLiteral explicit_builtin_types[] = {
                "bool", "char", "signed char", "unsigned char", "short",
                "unsigned short", "int", "unsigned int", "long",
                "unsigned long", "long long", "unsigned long long", "float",
                "double", "long double", "wchar_t", "char8_t", "char16_t",
                "char32_t"};
            bool explicit_builtin = false;
            for (const llvm::StringRef builtin : explicit_builtin_types) {
                if (replacement == builtin) {
                    explicit_builtin = true;
                    break;
                }
            }
            const llvm::StringRef qualified_text(replacement);
            const bool explicitly_rooted =
                qualified_text.starts_with("std::") ||
                qualified_text.starts_with("gui_forms::") ||
                qualified_text.starts_with("gui_drawing::");
            const llvm::StringRef spelling_text(replacement);
            if (replacement.empty()) {
                reason = "empty_type_spelling";
            } else if (spelling_text.contains("(lambda at") ||
                       spelling_text.contains("lambda_") ||
                       spelling_text.contains("anonymous")) {
                reason = "unnamed_closure_or_local_type";
            } else if (spelling_text.contains("__") ||
                       spelling_text.contains("std::__") ||
                       spelling_text.contains("__gnu") ||
                       spelling_text.contains("__cxx") ||
                       spelling_text.contains("__wrap_iter")) {
                reason = "implementation_private_type";
            } else if (ambiguous_nested_alias) {
                reason = "unqualified_nested_alias";
            } else if (!explicit_builtin && !explicitly_rooted) {
                reason = "unqualified_type_requires_owner_review";
            } else if (spelling_text.starts_with("const ") ||
                       spelling_text.starts_with("volatile ") ||
                       spelling_text.ends_with(" *") ||
                       spelling_text.ends_with("&")) {
                reason = "declarator_qualifier_requires_review";
            } else if (spelling_text.contains("auto") ||
                       spelling_text.contains("type-parameter") ||
                       spelling_text.contains("<dependent")) {
                reason = "dependent_type_spelling";
            } else if (spelling_text.contains('(') ||
                       spelling_text.contains(')') ||
                       spelling_text.contains('[') ||
                       spelling_text.contains(']')) {
                reason = "non_prefix_declarator";
            } else if (replacement.size() > 160U) {
                reason = "type_spelling_too_long_for_mechanical_rewrite";
            } else {
                safe = true;
                reason = "resolved_named_prefix_type";
            }
        }

        const std::string absolute_path =
            normalized_path(source_manager_.getFilename(spelling));
        const llvm::StringRef relative = llvm::StringRef(absolute_path).drop_front(
            normalized_root_.size() + 1U);
        const std::string path(relative);
        const clang::FileID file = source_manager_.getFileID(spelling);
        const std::uint64_t token_offset = source_manager_.getFileOffset(spelling);
        const std::string key = path + ":" + std::to_string(token_offset);
        const std::map<std::string, std::size_t>::iterator existing =
            auto_rewrite_indices_.find(key);
        if (existing != auto_rewrite_indices_.end()) {
            AutoTypeRewrite& prior = auto_rewrites_[(*existing).second];
            if (prior.replacement != replacement) {
                prior.safe = false;
                prior.reason = "translation_unit_type_conflict";
            }
            return;
        }
        auto_rewrite_indices_.emplace(key, auto_rewrites_.size());
        auto_rewrites_.push_back(AutoTypeRewrite{
            path, source_manager_.getBufferData(file).size(), token_offset,
            replacement, safe, reason});
    }

    [[nodiscard]] bool is_normative_location(
        clang::SourceLocation location) const {
        const clang::SourceLocation spelling =
            source_manager_.getSpellingLoc(location);
        if (!spelling.isValid() || source_manager_.isInSystemHeader(spelling)) {
            return false;
        }
        const std::string absolute_path =
            normalized_path(source_manager_.getFilename(spelling));
        return is_normative_path(absolute_path);
    }

private:
    [[nodiscard]] bool is_normative_path(const std::string& path) const {
        const llvm::StringRef value(path);
        if (!value.starts_with(normalized_root_ + "/")) return false;
        if (source_scope == "production" &&
            !contains_component(value, "/include/") &&
            !contains_component(value, "/src/")) {
            return false;
        }
        return !contains_component(value, "/third_party/") &&
            !contains_component(value, "/experiments/") &&
            !contains_component(value, "/build/") &&
            !contains_component(value, "/.build/");
    }

    [[nodiscard]] std::string enclosing_symbol(
        const clang::DynTypedNode& starting_node) const {
        clang::DynTypedNode current = starting_node;
        std::string nearest_named;
        for (unsigned depth = 0U; depth < 48U; ++depth) {
            const clang::DynTypedNodeList parents = context_.getParents(current);
            if (parents.empty()) break;
            bool advanced = false;
            for (const clang::DynTypedNode& parent : parents) {
                const clang::NamedDecl* named = parent.get<clang::NamedDecl>();
                if (named != nullptr && !(*named).isImplicit()) {
                    const std::string qualified = (*named).getQualifiedNameAsString();
                    if (nearest_named.empty()) nearest_named = qualified;
                    if (llvm::isa<clang::FunctionDecl>(named)) return qualified;
                }
                if (!advanced) {
                    current = parent;
                    advanced = true;
                }
            }
            if (!advanced) break;
        }
        return nearest_named;
    }

    [[nodiscard]] static bool is_tag_exception(
        llvm::StringRef enclosing) noexcept {
        static constexpr llvm::StringLiteral owners[] = {
            "gui_forms::Control", "gui_forms::ImageList",
            "gui_forms::ErrorProvider", "gui_forms::HelpProvider"};
        for (llvm::StringRef owner : owners) {
            const std::string tag = (owner + "::tag").str();
            const std::string set_tag = (owner + "::set_tag").str();
            const std::string clear_tag = (owner + "::clear_tag").str();
            const std::string tag_member = (owner + "::tag_").str();
            if (enclosing == tag || enclosing == set_tag ||
                enclosing == clear_tag || enclosing == tag_member) {
                return true;
            }
        }
        return enclosing == "gui_forms::showcase::initialize_showcase_runtime" ||
               enclosing == "file_manager_demoboard::set_product_surface" ||
               enclosing ==
                   "(anonymous namespace)::test_public_drawing_metrics_and_control_tag" ||
               enclosing ==
                   "(anonymous namespace)::test_error_semantics_geometry_rtl_and_lifetime" ||
               enclosing ==
                   "(anonymous namespace)::test_help_routes_f1_locally_then_to_provider_without_external_policy";
    }

    clang::ASTContext& context_;
    clang::SourceManager& source_manager_;
    std::vector<Finding>& findings_;
    std::set<std::string>& keys_;
    std::vector<PointerArrowRewrite>& pointer_rewrites_;
    std::set<std::string>& pointer_rewrite_keys_;
    std::vector<AutoTypeRewrite>& auto_rewrites_;
    std::map<std::string, std::size_t>& auto_rewrite_indices_;
    std::string normalized_root_;
};

class PolicyVisitor final
    : public clang::RecursiveASTVisitor<PolicyVisitor> {
public:
    PolicyVisitor(clang::ASTContext& context, std::vector<Finding>& findings,
                  std::set<std::string>& keys,
                  std::vector<PointerArrowRewrite>& pointer_rewrites,
                  std::set<std::string>& pointer_rewrite_keys,
                  std::vector<AutoTypeRewrite>& auto_rewrites,
                  std::map<std::string, std::size_t>& auto_rewrite_indices)
        : context_(context),
          collector_(context, findings, keys, pointer_rewrites,
                     pointer_rewrite_keys, auto_rewrites,
                     auto_rewrite_indices) {}

    [[nodiscard]] bool shouldVisitTemplateInstantiations() const noexcept {
        return false;
    }

    bool VisitAutoTypeLoc(clang::AutoTypeLoc location) {
        collector_.add_auto_type(location);
        return true;
    }

    bool VisitVarDecl(clang::VarDecl* declaration) {
        const clang::TypeSourceInfo* source_info =
            (*declaration).getTypeSourceInfo();
        if (source_info == nullptr) return true;
        for (clang::TypeLoc current = (*source_info).getTypeLoc();
             !current.isNull(); current = current.getNextTypeLoc()) {
            const clang::AutoTypeLoc automatic = current.getAs<clang::AutoTypeLoc>();
            if (automatic.isNull()) continue;
            collector_.add_auto_type(automatic, (*declaration).getType(),
                                     (*declaration).getLocation());
            break;
        }
        return true;
    }

    bool VisitLambdaExpr(clang::LambdaExpr* expression) {
        collector_.add("lambda", (*expression).getBeginLoc(), *expression,
                       "named_function_or_functor");
        return true;
    }

    bool VisitDecompositionDecl(clang::DecompositionDecl* declaration) {
        collector_.add("structured_binding", (*declaration).getLocation(),
                       *declaration, "explicit_named_bindings");
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr* expression) {
        if ((*expression).isArrow()) {
            collector_.add_pointer_arrow(*expression);
        }
        return true;
    }

    bool VisitCXXDependentScopeMemberExpr(
        clang::CXXDependentScopeMemberExpr* expression) {
        if ((*expression).isArrow()) {
            collector_.add_pointer_arrow(*expression);
        }
        return true;
    }

    bool VisitFunctionDecl(clang::FunctionDecl* declaration) {
        if ((*declaration).isImplicit()) return true;
        const clang::TypeSourceInfo* source_info =
            (*declaration).getTypeSourceInfo();
        if (source_info != nullptr) {
            const clang::SourceLocation function_location =
                (*declaration).getLocation();
            const unsigned function_offset = function_location.isValid()
                ? context_source_offset(function_location) : 0U;
            for (clang::TypeLoc current = (*source_info).getTypeLoc();
                 !current.isNull(); current = current.getNextTypeLoc()) {
                const clang::AutoTypeLoc automatic =
                    current.getAs<clang::AutoTypeLoc>();
                if (automatic.isNull()) continue;
                const unsigned auto_offset =
                    context_source_offset(automatic.getBeginLoc());
                if (function_offset != 0U && auto_offset < function_offset) {
                    collector_.add_auto_type(
                        automatic, (*declaration).getReturnType(),
                        function_location);
                }
                break;
            }
        }
        const clang::FunctionProtoType* prototype =
            (*(*declaration).getType()).getAs<clang::FunctionProtoType>();
        if (prototype != nullptr && (*prototype).hasTrailingReturn()) {
            collector_.add("trailing_return", (*declaration).getLocation(),
                           *declaration, "leading_explicit_return_type");
        }
        if ((*declaration).isConsteval()) {
            collector_.add("consteval", (*declaration).getLocation(), *declaration,
                           "retain_explicit_compile_time_execution", "admitted",
                           "O-002-consteval");
        }
        if ((*declaration).isDefaulted() && (*declaration).isOverloadedOperator()) {
            const clang::OverloadedOperatorKind operator_kind =
                (*declaration).getOverloadedOperator();
            if (operator_kind == clang::OO_EqualEqual ||
                operator_kind == clang::OO_ExclaimEqual ||
                operator_kind == clang::OO_Less ||
                operator_kind == clang::OO_LessEqual ||
                operator_kind == clang::OO_Greater ||
                operator_kind == clang::OO_GreaterEqual ||
                operator_kind == clang::OO_Spaceship) {
                collector_.add("defaulted_comparison", (*declaration).getLocation(),
                               *declaration, "explicit_needed_comparison");
            }
        }
        return true;
    }

    bool VisitIfStmt(clang::IfStmt* statement) {
        if ((*statement).isConstexpr()) {
            collector_.add("if_constexpr", (*statement).getIfLoc(), *statement,
                           "retain_named_template_folding", "admitted",
                           "O-004-if-constexpr");
        }
        return true;
    }

    bool VisitRequiresExpr(clang::RequiresExpr* expression) {
        collector_.add("requires_expression", (*expression).getRequiresKWLoc(),
                       *expression, "explicit_specialized_trait");
        return true;
    }

    bool VisitDecltypeTypeLoc(clang::DecltypeTypeLoc location) {
        collector_.add("decltype_expression", location.getBeginLoc(), location,
                       "named_private_trait_or_explicit_type");
        return true;
    }

    bool VisitCoroutineBodyStmt(clang::CoroutineBodyStmt* statement) {
        collector_.add("coroutine", (*statement).getBeginLoc(), *statement,
                       "explicit_state_machine");
        return true;
    }

    bool VisitCoawaitExpr(clang::CoawaitExpr* expression) {
        collector_.add("co_await", (*expression).getBeginLoc(), *expression,
                       "explicit_state_machine");
        return true;
    }

    bool VisitCoyieldExpr(clang::CoyieldExpr* expression) {
        collector_.add("co_yield", (*expression).getBeginLoc(), *expression,
                       "explicit_state_machine");
        return true;
    }

    bool VisitCoreturnStmt(clang::CoreturnStmt* statement) {
        collector_.add("co_return", (*statement).getBeginLoc(), *statement,
                       "explicit_state_machine");
        return true;
    }

    bool VisitDesignatedInitExpr(clang::DesignatedInitExpr* expression) {
        collector_.add("designated_initializer", (*expression).getBeginLoc(),
                       *expression, "plain_configuration_record_review",
                       "review", "O-007-classify-target");
        return true;
    }

    bool VisitConceptSpecializationExpr(
        clang::ConceptSpecializationExpr* expression) {
        collector_.add("concept_specialization", (*expression).getBeginLoc(),
                       *expression, "explicit_template_constraint_review",
                       "review", "O-009-concrete-review");
        return true;
    }

    bool VisitTypeLoc(clang::TypeLoc location) {
        if (!collector_.is_normative_location(location.getBeginLoc())) {
            return true;
        }
        const std::string type_name =
            location.getType().getCanonicalType().getAsString();
        const llvm::StringRef name(type_name);
        if (name.contains("std::any")) {
            collector_.add("std_any", location.getBeginLoc(), location,
                           "tag_only_or_explicit_type");
        }
        if (name.contains("std::ranges::") || name.contains("std::ranges")) {
            collector_.add("std_ranges", location.getBeginLoc(), location,
                           "concrete_architect_review");
        }
        if (name.contains("std::span")) {
            collector_.add("std_span", location.getBeginLoc(), location,
                           "retain_explicit_nonowning_range", "admitted",
                           "O-002-span");
        }
        return true;
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr* expression) {
        const clang::NamedDecl* declaration = (*expression).getFoundDecl();
        if (declaration == nullptr) return true;
        const std::string qualified = (*declaration).getQualifiedNameAsString();
        const llvm::StringRef name(qualified);
        if (name.starts_with("std::any_cast")) {
            collector_.add("std_any", (*expression).getLocation(), *expression,
                           "tag_only_or_explicit_type");
        }
        if (name.starts_with("std::ranges::")) {
            collector_.add("std_ranges", (*expression).getLocation(), *expression,
                           "concrete_architect_review");
        }
        if (name == "std::has_single_bit") {
            collector_.add("has_single_bit", (*expression).getLocation(),
                           *expression, "retain_clear_bit_predicate", "admitted",
                           "O-008-has-single-bit");
        }
        return true;
    }

private:
    [[nodiscard]] unsigned context_source_offset(
        clang::SourceLocation location) const {
        const clang::SourceManager& source_manager =
            collector_context().getSourceManager();
        const clang::SourceLocation spelling =
            source_manager.getSpellingLoc(location);
        return spelling.isValid() ? source_manager.getFileOffset(spelling) : 0U;
    }

    [[nodiscard]] clang::ASTContext& collector_context() const {
        return context_;
    }

    clang::ASTContext& context_;
    FindingCollector collector_;
};

class PolicyConsumer final : public clang::ASTConsumer {
public:
    PolicyConsumer(std::vector<Finding>& findings, std::set<std::string>& keys,
                   std::vector<PointerArrowRewrite>& pointer_rewrites,
                   std::set<std::string>& pointer_rewrite_keys,
                   std::vector<AutoTypeRewrite>& auto_rewrites,
                   std::map<std::string, std::size_t>& auto_rewrite_indices)
        : findings_(findings), keys_(keys),
          pointer_rewrites_(pointer_rewrites),
          pointer_rewrite_keys_(pointer_rewrite_keys),
          auto_rewrites_(auto_rewrites),
          auto_rewrite_indices_(auto_rewrite_indices) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        PolicyVisitor visitor(context, findings_, keys_, pointer_rewrites_,
                              pointer_rewrite_keys_, auto_rewrites_,
                              auto_rewrite_indices_);
        const bool traversed =
            visitor.TraverseDecl(context.getTranslationUnitDecl());
        if (!traversed) {
            llvm::errs() << "house-policy: AST traversal stopped early\n";
        }
    }

private:
    std::vector<Finding>& findings_;
    std::set<std::string>& keys_;
    std::vector<PointerArrowRewrite>& pointer_rewrites_;
    std::set<std::string>& pointer_rewrite_keys_;
    std::vector<AutoTypeRewrite>& auto_rewrites_;
    std::map<std::string, std::size_t>& auto_rewrite_indices_;
};

class PolicyAction final : public clang::ASTFrontendAction {
public:
    PolicyAction(std::vector<Finding>& findings, std::set<std::string>& keys,
                 std::vector<PointerArrowRewrite>& pointer_rewrites,
                 std::set<std::string>& pointer_rewrite_keys,
                 std::vector<AutoTypeRewrite>& auto_rewrites,
                 std::map<std::string, std::size_t>& auto_rewrite_indices)
        : findings_(findings), keys_(keys),
          pointer_rewrites_(pointer_rewrites),
          pointer_rewrite_keys_(pointer_rewrite_keys),
          auto_rewrites_(auto_rewrites),
          auto_rewrite_indices_(auto_rewrite_indices) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance&, llvm::StringRef) override {
        return std::make_unique<PolicyConsumer>(findings_, keys_,
                                                pointer_rewrites_,
                                                pointer_rewrite_keys_,
                                                auto_rewrites_,
                                                auto_rewrite_indices_);
    }

private:
    std::vector<Finding>& findings_;
    std::set<std::string>& keys_;
    std::vector<PointerArrowRewrite>& pointer_rewrites_;
    std::set<std::string>& pointer_rewrite_keys_;
    std::vector<AutoTypeRewrite>& auto_rewrites_;
    std::map<std::string, std::size_t>& auto_rewrite_indices_;
};

class PolicyActionFactory final : public clang::tooling::FrontendActionFactory {
public:
    PolicyActionFactory(std::vector<Finding>& findings,
                        std::set<std::string>& keys,
                        std::vector<PointerArrowRewrite>& pointer_rewrites,
                        std::set<std::string>& pointer_rewrite_keys,
                        std::vector<AutoTypeRewrite>& auto_rewrites,
                        std::map<std::string, std::size_t>& auto_rewrite_indices)
        : findings_(findings), keys_(keys),
          pointer_rewrites_(pointer_rewrites),
          pointer_rewrite_keys_(pointer_rewrite_keys),
          auto_rewrites_(auto_rewrites),
          auto_rewrite_indices_(auto_rewrite_indices) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<PolicyAction>(findings_, keys_, pointer_rewrites_,
                                              pointer_rewrite_keys_,
                                              auto_rewrites_,
                                              auto_rewrite_indices_);
    }

private:
    std::vector<Finding>& findings_;
    std::set<std::string>& keys_;
    std::vector<PointerArrowRewrite>& pointer_rewrites_;
    std::set<std::string>& pointer_rewrite_keys_;
    std::vector<AutoTypeRewrite>& auto_rewrites_;
    std::map<std::string, std::size_t>& auto_rewrite_indices_;
};

[[nodiscard]] llvm::json::Object finding_json(const Finding& finding) {
    llvm::json::Object object;
    object["path"] = finding.path;
    object["line"] = static_cast<std::int64_t>(finding.line);
    object["column"] = static_cast<std::int64_t>(finding.column);
    object["enclosingSymbol"] = finding.enclosing_symbol;
    object["constructKind"] = finding.construct_kind;
    object["replacementCategory"] = finding.replacement_category;
    object["disposition"] = finding.disposition;
    object["exceptionIdentifier"] = finding.exception_identifier;
    return object;
}

[[nodiscard]] bool write_ledger(const std::vector<Finding>& findings) {
    std::error_code error;
    llvm::raw_fd_ostream output(json_output, error);
    if (error) {
        llvm::errs() << "cannot write " << json_output << ": "
                     << error.message() << '\n';
        return false;
    }
    llvm::json::Array records;
    for (const Finding& finding : findings) {
        records.push_back(finding_json(finding));
    }
    llvm::json::Object root;
    root["schema"] = "gui.forms.house-policy-findings/v1";
    root["mode"] = policy_mode;
    root["sourceScope"] = source_scope;
    root["sourceRoot"] = normalized_path(source_root);
    root["findingCount"] = static_cast<std::int64_t>(findings.size());
    root["findings"] = std::move(records);
    output << llvm::formatv("{0:2}\n", llvm::json::Value(std::move(root)));
    return true;
}

[[nodiscard]] bool write_pointer_rewrites(
    const std::vector<PointerArrowRewrite>& rewrites) {
    if (pointer_rewrite_output.empty()) return true;
    std::error_code error;
    llvm::raw_fd_ostream output(pointer_rewrite_output, error);
    if (error) {
        llvm::errs() << "cannot write " << pointer_rewrite_output << ": "
                     << error.message() << '\n';
        return false;
    }
    llvm::json::Array records;
    for (const PointerArrowRewrite& rewrite : rewrites) {
        llvm::json::Object record;
        record["path"] = rewrite.path;
        record["fileSize"] = static_cast<std::int64_t>(rewrite.file_size);
        record["baseOffset"] = static_cast<std::int64_t>(rewrite.base_offset);
        record["operatorOffset"] =
            static_cast<std::int64_t>(rewrite.operator_offset);
        records.push_back(std::move(record));
    }
    llvm::json::Object root;
    root["schema"] = "gui.forms.pointer-arrow-rewrites/v1";
    root["sourceRoot"] = "gui_forms";
    root["rewriteCount"] = static_cast<std::int64_t>(rewrites.size());
    root["rewrites"] = std::move(records);
    output << llvm::formatv("{0:2}\n", llvm::json::Value(std::move(root)));
    return true;
}

[[nodiscard]] bool write_auto_rewrites(
    const std::vector<AutoTypeRewrite>& rewrites) {
    if (auto_rewrite_output.empty()) return true;
    std::error_code error;
    llvm::raw_fd_ostream output(auto_rewrite_output, error);
    if (error) {
        llvm::errs() << "cannot write " << auto_rewrite_output << ": "
                     << error.message() << '\n';
        return false;
    }
    llvm::json::Array records;
    std::size_t safe_count = 0U;
    for (const AutoTypeRewrite& rewrite : rewrites) {
        llvm::json::Object record;
        record["path"] = rewrite.path;
        record["fileSize"] = static_cast<std::int64_t>(rewrite.file_size);
        record["tokenOffset"] = static_cast<std::int64_t>(rewrite.token_offset);
        record["replacement"] = rewrite.replacement;
        record["safe"] = rewrite.safe;
        record["reason"] = rewrite.reason;
        records.push_back(std::move(record));
        if (rewrite.safe) ++safe_count;
    }
    llvm::json::Object root;
    root["schema"] = "gui.forms.auto-type-rewrites/v1";
    root["sourceRoot"] = "gui_forms";
    root["candidateCount"] = static_cast<std::int64_t>(rewrites.size());
    root["safeCount"] = static_cast<std::int64_t>(safe_count);
    root["rewrites"] = std::move(records);
    output << llvm::formatv("{0:2}\n", llvm::json::Value(std::move(root)));
    return true;
}

} // namespace

int main(int argc, const char** argv) {
    llvm::Expected<clang::tooling::CommonOptionsParser> options =
        clang::tooling::CommonOptionsParser::create(
            argc, argv, policy_category, llvm::cl::OneOrMore);
    if (!options) {
        llvm::errs() << llvm::toString(options.takeError()) << '\n';
        return EXIT_FAILURE;
    }
    if (policy_mode != "inventory" && policy_mode != "closure") {
        llvm::errs() << "unsupported --mode: " << policy_mode << '\n';
        return EXIT_FAILURE;
    }
    if (source_scope != "production" && source_scope != "first-party") {
        llvm::errs() << "unsupported --source-scope: " << source_scope << '\n';
        return EXIT_FAILURE;
    }

    std::vector<Finding> findings;
    std::set<std::string> keys;
    std::vector<PointerArrowRewrite> pointer_rewrites;
    std::set<std::string> pointer_rewrite_keys;
    std::vector<AutoTypeRewrite> auto_rewrites;
    std::map<std::string, std::size_t> auto_rewrite_indices;
    clang::tooling::ClangTool tool((*options).getCompilations(),
                                   (*options).getSourcePathList());
    PolicyActionFactory factory(findings, keys, pointer_rewrites,
                                pointer_rewrite_keys, auto_rewrites,
                                auto_rewrite_indices);
    const int tool_result = tool.run(&factory);
    if (tool_result != 0) return tool_result;

    std::sort(findings.begin(), findings.end(), finding_less);
    std::sort(pointer_rewrites.begin(), pointer_rewrites.end(),
              pointer_rewrite_less);
    std::sort(auto_rewrites.begin(), auto_rewrites.end(),
              auto_type_rewrite_less);
    if (!write_ledger(findings)) return EXIT_FAILURE;
    if (!write_pointer_rewrites(pointer_rewrites)) return EXIT_FAILURE;
    if (!write_auto_rewrites(auto_rewrites)) return EXIT_FAILURE;

    std::size_t violations = 0U;
    for (const Finding& finding : findings) {
        if (finding.disposition == "violation") ++violations;
        if (!quiet) {
            llvm::errs() << finding.path << ':' << finding.line << ':'
                         << finding.column << ": " << finding.disposition
                         << ": " << finding.construct_kind << " in "
                         << finding.enclosing_symbol << " ["
                         << finding.replacement_category << "]\n";
        }
    }
    llvm::outs() << "house-policy: " << findings.size() << " findings, "
                 << violations << " violations\n";
    if (policy_mode == "closure" && violations != 0U) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
