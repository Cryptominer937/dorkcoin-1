// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_THEMEMANAGER_H
#define BITCOIN_QT_THEMEMANAGER_H

#include <QColor>
#include <QObject>
#include <QPixmap>
#include <QString>

class QApplication;

/**
 * ThemeManager owns the semantic token architecture for the Dorkcoin Qt GUI.
 *
 * Every theme defines a fixed set of semantic tokens (window and elevated
 * surfaces, primary/muted text, borders, focus rings, interactive accent
 * states, disabled and tooltip tokens). A single application-wide stylesheet
 * is regenerated from those tokens whenever the theme changes, so all widgets
 * inherit the active theme without any per-widget hardcoded colors.
 *
 * The active theme is persisted in QSettings. When nothing is saved yet, the
 * user's OS preference (high contrast or dark mode) is detected and applied,
 * falling back to Light Mode. The theme is applied on the QApplication before
 * the first window is shown, so there is no flash of unstyled colors.
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    /** Available themes. Order matches the Display tab combo box items. */
    enum ThemeId {
        ThemeLight,
        ThemeDark,
        ThemeHighContrast,
        ThemeOcean,
        ThemeForest,
        ThemeSunset,
        ThemeCount
    };

    /** Semantic color tokens consumed by the generated stylesheet. */
    struct ThemeColors {
        QColor window;           // main window/surface background
        QColor panel;            // elevated surfaces (frames, group boxes, toolbars)
        QColor panelHover;       // subtle hover surface for panels/toolbar items
        QColor input;            // text fields, combo boxes, spin boxes
        QColor text;             // primary text
        QColor textMuted;        // secondary/placeholder text
        QColor textInverted;     // text drawn on top of the primary accent
        QColor border;           // neutral borders
        QColor borderFocus;      // focus rings and active borders
        QColor primary;          // primary accent
        QColor primaryHover;     // primary accent hovered
        QColor primaryActive;    // primary accent pressed/active
        QColor success;          // positive state accent
        QColor warning;          // warning state accent
        QColor error;            // error state accent
        QColor disabledBg;       // disabled control background
        QColor disabledText;     // disabled control text
        QColor tooltipBg;        // tooltip background
        QColor tooltipText;      // tooltip text
        int borderWidth;         // default border thickness in pixels
        int focusWidth;          // focus ring thickness in pixels
    };

    static ThemeManager *instance();
    ~ThemeManager();

    /** Load the saved theme (or detect the OS theme) and apply it to qApp.
        Must be called once the QApplication exists, before any window is shown. */
    void initialize();

    ThemeId currentTheme() const { return m_theme; }
    const QString &themeName(ThemeId theme) const;
    const ThemeColors &themeColors(ThemeId theme) const { return m_palettes[theme]; }
    const ThemeColors &currentColors() const { return m_palettes[m_theme]; }

    /** Detect the OS-preferred theme (high contrast, dark mode or light). */
    ThemeId detectSystemTheme() const;

    /** Apply a theme to the application: regenerates the stylesheet and notifies listeners. */
    void applyTheme(ThemeId theme);

    /** Small 16x16 color swatch of a theme's primary color (for the selector). */
    QPixmap themeSwatch(ThemeId theme) const;

Q_SIGNALS:
    void themeChanged(ThemeId theme);

private:
    explicit ThemeManager(QObject *parent = 0);
    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;

    ThemeId loadSavedTheme();
    void persistTheme(ThemeId theme);
    QString buildStylesheet(const ThemeColors &c) const;
    void applyStylesheet(const ThemeColors &c);

    ThemeColors paletteLight() const;
    ThemeColors paletteDark() const;
    ThemeColors paletteHighContrast() const;
    ThemeColors paletteOcean() const;
    ThemeColors paletteForest() const;
    ThemeColors paletteSunset() const;

    ThemeId m_theme;
    ThemeColors m_palettes[ThemeCount];
    QString m_themeNames[ThemeCount];
};

#endif // BITCOIN_QT_THEMEMANAGER_H
