#pragma once

namespace gui_forms::detail {

class Revocable {
public:
    virtual ~Revocable() = default;
    virtual void disconnect() noexcept = 0;
    [[nodiscard]] virtual bool connected() const noexcept = 0;
};

} // namespace gui_forms::detail
