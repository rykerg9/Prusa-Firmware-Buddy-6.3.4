#pragma once

#include <MItem_menus.hpp>
#include <WinMenuContainer.hpp>
#include <window_menu_adv.hpp>
#include <screen_menu.hpp>

#include <filament_list.hpp>
#include <i18n.h>
#include <dynamic_index_mapping.hpp>
#include <meta_utils.hpp>
#include <multi_filament_change.hpp>
#include <gui/menu_item/menu_item_select_menu.hpp>

namespace multi_filament_change {

class MI_ActionSelect : public MenuItemSelectMenu {

public:
    MI_ActionSelect(uint8_t tool_ix);

    void set_config(const ConfigItem &set);
    ConfigItem config() const;

    int item_count() const final;
    void build_item_text(int index, const std::span<char> &buffer) const final;

private:
    static constexpr auto items = std::to_array<DynamicIndexMappingRecord<Action>>({
        Action::keep,
        { Action::change, DynamicIndexMappingType::dynamic_section },
        { Action::unload, DynamicIndexMappingType::optional_item },
    });

private:
    const uint8_t tool_ix;
    bool has_filament_loaded = false;
    std::optional<Color> color;

    StringViewUtf8Parameters<2> label_params;
    DynamicIndexMapping<items> index_mapping;
    FilamentListStorage filament_list;
};

class MI_ApplyChanges : public IWindowMenuItem {
public:
    MI_ApplyChanges();

protected:
    virtual void click(IWindowMenu &) override;
};

template <typename>
struct MenuMultiFilamentChange__;

template <size_t... ix>
struct MenuMultiFilamentChange__<std::index_sequence<ix...>> {
    using Container = WinMenuContainer<MI_RETURN,
        WithConstructorArgs<MI_ActionSelect, ix>...,
        MI_ApplyChanges>;
};

using MenuMultiFilamentChange_ = MenuMultiFilamentChange__<std::make_index_sequence<multi_filament_change::tool_count>>;

class MenuMultiFilamentChange : public WindowMenu {

public:
    MenuMultiFilamentChange(window_t *parent, const Rect16 &rect);

public:
    inline bool is_carrying_out_changes() const {
        return is_carrying_out_changes_;
    }

    void set_configuration(const MultiFilamentChangeConfig &set);

protected:
    void windowEvent(window_t *sender, GUI_event_t event, void *param) override;

private:
    void carry_out_changes();

private:
    MenuMultiFilamentChange_::Container container;
    bool is_carrying_out_changes_ = false;
};

} // namespace multi_filament_change

/**
 * @brief Change filament in all tools.
 */
class ScreenChangeAllFilaments : public ScreenMenuBase<multi_filament_change::MenuMultiFilamentChange> {
public:
    ScreenChangeAllFilaments();
};

class DialogChangeAllFilaments final : public IDialog {
public:
    /// Executes the "change all filaments" dialog
    /// \param initial_config sets up the pre-selected configuration for the menu
    /// \param exit_on_media if true, closes the dialog on media removed/error
    /// \returns true if extited because of media error/disconnect
    static bool exec(const MultiFilamentChangeConfig &initial_config, bool exit_on_media = false);

protected:
    void windowEvent(window_t *sender, GUI_event_t event, void *param) override;

private:
    DialogChangeAllFilaments(const MultiFilamentChangeConfig &initial_configuration);

private:
    window_header_t header;
    WindowExtendedMenu<multi_filament_change::MenuMultiFilamentChange> menu;

private:
    bool exit_on_media = false;
    bool exited_by_media = false;
};
