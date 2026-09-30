#pragma once

#include "file_manager/document_picker.hpp"

#include "gui_forms/gui_forms.hpp"

#include <memory>
#include <vector>

namespace file_manager {

// GUI.Forms composition for the bounded picker package. Host applications own
// the top-level window/sheet and selection grant; this view owns only the
// picker controls and semantic interaction.
class DocumentPickerView final {
  public:
    explicit DocumentPickerView(DocumentPickerRequest request);
    ~DocumentPickerView();

    DocumentPickerView(const DocumentPickerView&) = delete;
    DocumentPickerView& operator=(const DocumentPickerView&) = delete;

    [[nodiscard]] std::shared_ptr<gui_forms::Control> root_control() const;
    [[nodiscard]] FileSelectionController& controller() noexcept;
    [[nodiscard]] const FileSelectionController& controller() const noexcept;
    [[nodiscard]] gui_forms::Event<const DocumentPickerResult&>& completed() noexcept;

    void set_orchestrator_session_valid(bool valid);
    void set_authority_valid(bool valid);
    void cancel();
    void confirm_overwrite();
    // Call after constructing the host-owned Window, and on each presentation.
    void attach_dialog(gui_forms::Window& window);
    void present(const std::filesystem::path& initial_location);

  private:
    void populate_options();
    void connect_callbacks();
    void location_changed(const std::optional<std::size_t> index);
    void name_filter_committed(const std::string& value);
    void home_clicked(gui_forms::ButtonBase&);
    void up_clicked(gui_forms::ButtonBase&);
    void path_committed(const std::string& value);
    void path_cancelled();
    void selection_changed(const gui_forms::ObjectSelectionChange&);
    void item_activated(const std::string& stable_id);
    void filter_changed(const std::optional<std::size_t> index);
    void hidden_changed(const bool checked);
    void filename_changed(const std::string& value);
    void filename_committed(const std::string&);
    void accept_clicked(gui_forms::ButtonBase&);
    void cancel_clicked(gui_forms::ButtonBase&);
    void reload();
    void update_status();
    void navigate_path(std::filesystem::path path);
    void accept(bool overwrite_confirmed = false);
    void publish(DocumentPickerResult result);
    void publish_pending_completion();

    FileSelectionController controller_;
    std::shared_ptr<gui_forms::ScaledPanel> root_{};
    std::shared_ptr<gui_forms::Control> title_bar_{};
    std::shared_ptr<gui_forms::Label> title_{};
    std::shared_ptr<gui_forms::Label> subtitle_{};
    std::shared_ptr<gui_forms::Control> navigation_bar_{};
    std::shared_ptr<gui_forms::Button> back_to_root_{};
    std::shared_ptr<gui_forms::Button> up_{};
    std::shared_ptr<gui_forms::TextBox> path_{};
    std::shared_ptr<gui_forms::ComboBox> locations_{};
    std::shared_ptr<gui_forms::TextBox> name_filter_{};
    std::shared_ptr<gui_forms::ObjectView> objects_{};
    std::shared_ptr<gui_forms::ComboBox> filter_{};
    std::shared_ptr<gui_forms::CheckBox> hidden_{};
    std::shared_ptr<gui_forms::TextBox> filename_{};
    std::shared_ptr<gui_forms::Label> status_{};
    std::shared_ptr<gui_forms::Button> accept_{};
    std::shared_ptr<gui_forms::Button> cancel_{};
    std::vector<gui_forms::SubscriptionToken> subscriptions_{};
    gui_forms::Event<const DocumentPickerResult&> completed_{};
    bool reloading_{};
    bool finished_{};
    std::optional<DocumentPickerResult> pending_completion_{};
};

} // namespace file_manager
