#pragma once

#include "file_manager/document_picker.hpp"

#include "gui_forms/gui_forms.hpp"

#include <memory>
#include <vector>

namespace file_manager {

// GUI.Forms composition for the bounded picker package. Host applications own
// the top-level window/sheet and Orchestrator session; this view owns only the
// picker controls and semantic interaction.
class DocumentPickerView final {
public:
    explicit DocumentPickerView(DocumentPickerRequest request);
    ~DocumentPickerView() = default;

    DocumentPickerView(const DocumentPickerView&) = delete;
    DocumentPickerView& operator=(const DocumentPickerView&) = delete;

    [[nodiscard]] std::shared_ptr<gui_forms::Control> root_control() const;
    [[nodiscard]] FileSelectionController& controller() noexcept;
    [[nodiscard]] const FileSelectionController& controller() const noexcept;
    [[nodiscard]] gui_forms::Event<const DocumentPickerResult&>& completed()
        noexcept;

    void set_orchestrator_session_valid(bool valid);
    void set_authority_valid(bool valid);
    void cancel();
    void confirm_overwrite();
    // Call after constructing the host-owned Window, and on each presentation.
    void attach_dialog(gui_forms::Window& window);
    void present(const std::filesystem::path& initial_location);

private:
    void reload();
    void update_status();
    void navigate_path(std::filesystem::path path);
    void accept(bool overwrite_confirmed = false);
    void publish(DocumentPickerResult result);

    FileSelectionController controller_;
    std::shared_ptr<gui_forms::ScaledPanel> root_;
    std::shared_ptr<gui_forms::Control> title_bar_;
    std::shared_ptr<gui_forms::Label> title_;
    std::shared_ptr<gui_forms::Label> subtitle_;
    std::shared_ptr<gui_forms::Control> navigation_bar_;
    std::shared_ptr<gui_forms::Button> back_to_root_;
    std::shared_ptr<gui_forms::Button> up_;
    std::shared_ptr<gui_forms::TextBox> path_;
    std::shared_ptr<gui_forms::ComboBox> locations_;
    std::shared_ptr<gui_forms::TextBox> name_filter_;
    std::shared_ptr<gui_forms::ObjectView> objects_;
    std::shared_ptr<gui_forms::ComboBox> filter_;
    std::shared_ptr<gui_forms::CheckBox> hidden_;
    std::shared_ptr<gui_forms::TextBox> filename_;
    std::shared_ptr<gui_forms::Label> status_;
    std::shared_ptr<gui_forms::Button> accept_;
    std::shared_ptr<gui_forms::Button> cancel_;
    std::vector<gui_forms::SubscriptionToken> subscriptions_;
    gui_forms::Event<const DocumentPickerResult&> completed_;
    bool reloading_{};
    bool finished_{};
};

} // namespace file_manager
