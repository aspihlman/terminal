// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

#include "Tab.h"
#include "CascadiaSettings.h"
#include "App.g.h"
#include "App.base.h"
#include "ScopedResourceLoader.h"
#include "../../cascadia/inc/cppwinrt_utils.h"

#include <winrt/Microsoft.Terminal.TerminalControl.h>
#include <winrt/Microsoft.Terminal.TerminalConnection.h>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>

#include <winrt/Windows.ApplicationModel.DataTransfer.h>

namespace winrt::TerminalApp::implementation
{
    struct App : AppT2<App>
    {
    public:
        App();

        Windows::UI::Xaml::UIElement GetRoot() noexcept;

        void Create();
        void LoadSettings();

        Windows::Foundation::Point GetLaunchDimensions(uint32_t dpi);
        bool GetShowTabsInTitlebar();

        ~App();

        hstring GetTitle();

        // -------------------------------- WinRT Events ---------------------------------
        DECLARE_EVENT(TitleChanged, _titleChangeHandlers, winrt::Microsoft::Terminal::TerminalControl::TitleChangedEventArgs);
        DECLARE_EVENT(LastTabClosed, _lastTabClosedHandlers, winrt::TerminalApp::LastTabClosedEventArgs);
        DECLARE_EVENT_WITH_TYPED_EVENT_HANDLER(SetTitleBarContent, _setTitleBarContentHandlers, TerminalApp::App, winrt::Windows::UI::Xaml::UIElement);
        DECLARE_EVENT_WITH_TYPED_EVENT_HANDLER(RequestedThemeChanged, _requestedThemeChangedHandlers, TerminalApp::App, winrt::Windows::UI::Xaml::ElementTheme);

    private:
        // If you add controls here, but forget to null them either here or in
        // the ctor, you're going to have a bad time. It'll mysteriously fail to
        // activate the app.
        // ALSO: If you add any UIElements as roots here, make sure they're
        // updated in _ApplyTheme. The two roots currently are _root and _tabRow
        // (which is a root when the tabs are in the titlebar.)
        Windows::UI::Xaml::Controls::Control _root{ nullptr };
        Microsoft::UI::Xaml::Controls::TabView _tabView{ nullptr };
        TerminalApp::TabRowControl _tabRow{ nullptr };
        Windows::UI::Xaml::Controls::Grid _tabContent{ nullptr };
        Windows::UI::Xaml::Controls::SplitButton _newTabButton{ nullptr };

        std::vector<std::shared_ptr<Tab>> _tabs;

        std::unique_ptr<::TerminalApp::CascadiaSettings> _settings;

        HRESULT _settingsLoadedResult;

        bool _loadedInitialSettings;
        std::shared_mutex _dialogLock;

        ScopedResourceLoader _resourceLoader;

        wil::unique_folder_change_reader_nothrow _reader;

        std::atomic<bool> _settingsReloadQueued{ false };

        void _CreateNewTabFlyout();

        fire_and_forget _ShowDialog(const winrt::Windows::Foundation::IInspectable& titleElement,
                                    const winrt::Windows::Foundation::IInspectable& contentElement,
                                    const winrt::hstring& closeButtonText);
        void _ShowOkDialog(const winrt::hstring& titleKey, const winrt::hstring& contentKey);
        void _ShowAboutDialog();

        [[nodiscard]] HRESULT _TryLoadSettings(const bool saveOnLoad) noexcept;
        void _LoadSettings();
        void _OpenSettings();

        void _HookupKeyBindings(TerminalApp::AppKeyBindings bindings) noexcept;

        void _RegisterSettingsChange();
        fire_and_forget _DispatchReloadSettings();
        void _ReloadSettings();

        void _SettingsButtonOnClick(const IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs& eventArgs);
        void _FeedbackButtonOnClick(const IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs& eventArgs);
        void _AboutButtonOnClick(const IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs& eventArgs);

        void _UpdateTabView();
        void _UpdateTabIcon(std::shared_ptr<Tab> tab);
        void _UpdateTitle(std::shared_ptr<Tab> tab);

        void _RegisterTerminalEvents(Microsoft::Terminal::TerminalControl::TermControl term, std::shared_ptr<Tab> hostingTab);

        void _CreateNewTabFromSettings(GUID profileGuid, winrt::Microsoft::Terminal::Settings::TerminalSettings settings);

        void _OpenNewTab(std::optional<int> profileIndex);
        void _DuplicateTabViewItem();
        void _CloseFocusedTab();
        void _CloseFocusedPane();
        void _SelectNextTab(const bool bMoveRight);
        void _SelectTab(const int tabIndex);

        void _SetFocusedTabIndex(int tabIndex);
        int _GetFocusedTabIndex() const;

        void _Scroll(int delta);
        void _CopyText(const bool trimTrailingWhitespace);
        void _PasteText();
        void _SplitVertical(const std::optional<GUID>& profileGuid);
        void _SplitHorizontal(const std::optional<GUID>& profileGuid);
        void _SplitPane(const Pane::SplitState splitType, const std::optional<GUID>& profileGuid);

        // Todo: add more event implementations here
        // MSFT:20641986: Add keybindings for New Window
        void _ScrollPage(int delta);
        void _ResizePane(const Direction& direction);
        void _MoveFocus(const Direction& direction);

        void _OnLoaded(const IInspectable& sender, const Windows::UI::Xaml::RoutedEventArgs& eventArgs);
        void _OnTabSelectionChanged(const IInspectable& sender, const Windows::UI::Xaml::Controls::SelectionChangedEventArgs& eventArgs);
        void _OnTabClosing(const IInspectable& sender, const Microsoft::UI::Xaml::Controls::TabViewTabClosingEventArgs& eventArgs);
        void _OnTabItemsChanged(const IInspectable& sender, const Windows::Foundation::Collections::IVectorChangedEventArgs& eventArgs);
        void _OnTabClick(const IInspectable& sender, const Windows::UI::Xaml::Input::PointerRoutedEventArgs& eventArgs);
        void _OnContentSizeChanged(const IInspectable& sender, Windows::UI::Xaml::SizeChangedEventArgs const& e);

        void _RemoveTabViewItem(const IInspectable& tabViewItem);

        void _ApplyTheme(const Windows::UI::Xaml::ElementTheme& newTheme);

        static Windows::UI::Xaml::Controls::IconElement _GetIconFromProfile(const ::TerminalApp::Profile& profile);

        winrt::Microsoft::Terminal::TerminalControl::TermControl _GetFocusedControl();

        void _CopyToClipboardHandler(const winrt::hstring& copiedData);
        void _PasteFromClipboardHandler(const IInspectable& sender, const Microsoft::Terminal::TerminalControl::PasteFromClipboardEventArgs& eventArgs);

        static void _SetAcceleratorForMenuItem(Windows::UI::Xaml::Controls::MenuFlyoutItem& menuItem, const winrt::Microsoft::Terminal::Settings::KeyChord& keyChord);
    };
}

namespace winrt::TerminalApp::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}
