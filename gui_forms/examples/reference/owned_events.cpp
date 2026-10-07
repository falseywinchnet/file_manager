#include <gui_forms/basic_controls.hpp>
#include <gui_forms/component.hpp>
#include <gui_forms/event.hpp>

#include <memory>

namespace gui_forms::examples {

// Actions owns the subscription. The retained parent continues to own the
// button; this connection adds no visual ownership or scheduler work.
class Actions final : public Component {
public:
    explicit Actions(Button& run) {
        on(run.clicked(), *this, &Actions::on_run);
    }

    [[nodiscard]] unsigned runs() const noexcept { return runs_; }

private:
    void on_run(ButtonBase&) { ++runs_; }

    unsigned runs_{};
};

} // namespace gui_forms::examples

int main() {
    const std::shared_ptr<gui_forms::Button> run =
        gui_forms::make_control<gui_forms::Button>(
            gui_forms::StableId("example.run"), "Run");
    gui_forms::examples::Actions actions(*run);
    (*run).perform_click();
    if (actions.runs() != 1U) {
        return 1;
    }
    actions.dispose();
    (*run).perform_click();
    if (actions.runs() != 1U) {
        return 1;
    }
    return 0;
}
