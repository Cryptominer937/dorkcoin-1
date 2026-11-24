// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "thememanager.h"

#include "platformstyle.h"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QPalette>
#include <QPixmap>
#include <QSettings>
#include <QString>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

ThemeManager *ThemeManager::instance()
{
    static ThemeManager *inst = 0;
    if (!inst)
        inst = new ThemeManager(QApplication::instance());
    return inst;
}

void ThemeManager::setPlatformStyle(PlatformStyle *style)
{
    m_platformStyle = style;
    if (m_platformStyle)
        m_platformStyle->setSingleColor(m_palettes[m_theme].primary);
}

ThemeManager::ThemeManager(QObject *parent) :
    QObject(parent),
    m_theme(ThemeLight),
    m_platformStyle(0)
{
    m_themeNames[ThemeLight] = tr("Light Mode");
    m_themeNames[ThemeDark] = tr("Dark Mode");
    m_themeNames[ThemeHighContrast] = tr("High Contrast");
    m_themeNames[ThemeOcean] = tr("Ocean");
    m_themeNames[ThemeForest] = tr("Forest");
    m_themeNames[ThemeSunset] = tr("Sunset");

    m_palettes[ThemeLight] = paletteLight();
    m_palettes[ThemeDark] = paletteDark();
    m_palettes[ThemeHighContrast] = paletteHighContrast();
    m_palettes[ThemeOcean] = paletteOcean();
    m_palettes[ThemeForest] = paletteForest();
    m_palettes[ThemeSunset] = paletteSunset();
}

ThemeManager::~ThemeManager()
{
}

const QString &ThemeManager::themeName(ThemeId theme) const
{
    return m_themeNames[theme];
}

void ThemeManager::initialize()
{
    ThemeId theme = loadSavedTheme();
    if (theme < 0 || theme >= ThemeCount)
        theme = detectSystemTheme();
    // Apply directly: m_theme is still ThemeLight from construction, so
    // applyTheme()'s m_theme == theme guard would skip the first apply.
    m_theme = theme;
    applyStylesheet(m_palettes[m_theme]);
    if (m_platformStyle)
        m_platformStyle->setSingleColor(m_palettes[m_theme].primary);
    persistTheme(theme);
    Q_EMIT themeChanged(m_theme);
}

ThemeManager::ThemeId ThemeManager::detectSystemTheme() const
{
#if defined(Q_OS_WIN)
    HIGHCONTRASTW hc;
    hc.cbSize = sizeof(HIGHCONTRASTW);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, FALSE) &&
        (hc.dwFlags & HCF_HIGHCONTRASTON))
    {
        return ThemeHighContrast;
    }
#endif
    const QColor window = QApplication::palette().color(QPalette::Window);
    return window.lightness() < 128 ? ThemeDark : ThemeLight;
}

void ThemeManager::applyTheme(ThemeId theme)
{
    if (theme < 0 || theme >= ThemeCount)
        theme = ThemeLight;
    if (m_theme == theme)
        return;
    m_theme = theme;
    applyStylesheet(m_palettes[m_theme]);
    if (m_platformStyle)
        m_platformStyle->setSingleColor(m_palettes[m_theme].primary);
    Q_EMIT themeChanged(m_theme);
}

void ThemeManager::applyStylesheet(const ThemeColors &c)
{
    QCoreApplication *app = QCoreApplication::instance();
    QApplication *qapp = qobject_cast<QApplication *>(app);
    if (!qapp)
        return;
    qapp->setStyleSheet(buildStylesheet(c));
}

QPixmap ThemeManager::themeSwatch(ThemeId theme) const
{
    QPixmap swatch(16, 16);
    swatch.fill(m_palettes[theme].primary);
    return swatch;
}

ThemeManager::ThemeId ThemeManager::loadSavedTheme()
{
    QSettings settings;
    int saved = settings.value("nTheme", -1).toInt();
    return (ThemeId)saved;
}

void ThemeManager::persistTheme(ThemeId theme)
{
    QSettings settings;
    settings.setValue("nTheme", (int)theme);
    settings.sync();
}

QString ThemeManager::buildStylesheet(const ThemeColors &c) const
{
    QString bgWidgetBg;
    if (m_theme == ThemeHighContrast)
        bgWidgetBg = c.window.name();
    else
        bgWidgetBg = QString("rgba(%1,%2,%3,%4)").arg(c.panel.red()).arg(c.panel.green()).arg(c.panel.blue()).arg(200);

    return QString(
        "QToolTip {"
        "  color: %19;"
        "  background-color: %18;"
        "  border: %20px solid %8;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}"
        "#bgWidget {"
        "  background-color: %22;"
        "}"
        "#contentWidget {"
        "  background-color: %2;"
        "  border: %20px solid %8;"
        "  border-radius: 6px;"
        "}"
        "#contentWidget QLabel {"
        "  color: %5;"
        "}"
        "QWidget {"
        "  color: %5;"
        "  background-color: %1;"
        "  selection-color: %7;"
        "  selection-background-color: %10;"
        "}"
        "QMainWindow, QDialog {"
        "  background-color: %1;"
        "}"
        "QStatusBar {"
        "  background-color: %2;"
        "  color: %5;"
        "  border-top: %20px solid %8;"
        "}"
        "QMenuBar {"
        "  background-color: %2;"
        "  color: %5;"
        "  border-bottom: %20px solid %8;"
        "}"
        "QMenuBar::item:selected {"
        "  background-color: %3;"
        "  border: %20px solid %9;"
        "  border-radius: 4px;"
        "}"
        "QMenu {"
        "  background-color: %2;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-radius: 6px;"
        "}"
        "QMenu::item {"
        "  padding: 5px 20px 5px 15px;"
        "  border: %20px solid transparent;"
        "  border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: %10;"
        "  color: %7;"
        "  border: %20px solid %9;"
        "}"
        "QMenu::separator {"
        "  background-color: %8;"
        "  height: %20px;"
        "  margin: 4px 0px;"
        "}"
        "QFrame, QGroupBox {"
        "  background-color: %2;"
        "  border: %20px solid %8;"
        "  border-radius: 6px;"
        "}"
        "QGroupBox::title {"
        "  color: %5;"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 4px;"
        "}"
        "QScrollBar:vertical {"
        "  background-color: %2;"
        "  width: 14px;"
        "  margin: 0px;"
        "  border: %20px solid %8;"
        "  border-radius: 7px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background-color: %6;"
        "  min-height: 24px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background-color: %5;"
        "}"
        "QScrollBar:horizontal {"
        "  background-color: %2;"
        "  height: 14px;"
        "  margin: 0px;"
        "  border: %20px solid %8;"
        "  border-radius: 7px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background-color: %6;"
        "  min-width: 24px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "  background-color: %5;"
        "}"
        "QTabWidget::pane {"
        "  border: %20px solid %8;"
        "  background-color: %2;"
        "  border-radius: 6px;"
        "}"
        "QTabBar::tab {"
        "  background-color: %2;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-bottom-color: %8;"
        "  border-top-left-radius: 5px;"
        "  border-top-right-radius: 5px;"
        "  min-width: 70px;"
        "  padding: 7px 14px;"
        "  margin-right: 3px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: %1;"
        "  border-bottom: %20px solid %10;"
        "  color: %10;"
        "}"
        "QTabBar::tab:selected:hover {"
        "  color: %11;"
        "}"
        "QTabBar::tab:hover {"
        "  background-color: %3;"
        "}"
        "QLineEdit, QValidatedLineEdit, QValueComboBox, QComboBox, QSpinBox {"
        "  background-color: %4;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-radius: 5px;"
        "  padding: 4px 8px;"
        "  min-height: 26px;"
        "}"
        "QLineEdit:focus, QValidatedLineEdit:focus, QValueComboBox:focus, QComboBox:focus, QSpinBox:focus {"
        "  border: %21px solid %9;"
        "  border-radius: 5px;"
        "}"
        "QLineEdit:disabled, QValidatedLineEdit:disabled, QValueComboBox:disabled, QComboBox:disabled, QSpinBox:disabled {"
        "  background-color: %16;"
        "  color: %17;"
        "}"
        "QComboBox::drop-down, QSpinBox::down-button, QSpinBox::up-button {"
        "  border: none;"
        "  width: 16px;"
        "}"
        "QPushButton {"
        "  background-color: %2;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-radius: 5px;"
        "  padding: 5px 16px;"
        "  min-height: 26px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %3;"
        "  border-color: %9;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %12;"
        "}"
        "QPushButton:default {"
        "  background-color: %10;"
        "  color: %7;"
        "  border: %20px solid %10;"
        "}"
        "QPushButton:default:hover {"
        "  background-color: %11;"
        "}"
        "QPushButton:default:pressed {"
        "  background-color: %12;"
        "}"
        "QPushButton:disabled {"
        "  background-color: %16;"
        "  color: %17;"
        "  border: %20px solid %16;"
        "}"
        "QCheckBox::indicator, QRadioButton::indicator {"
        "  width: 16px;"
        "  height: 16px;"
        "  border: %20px solid %8;"
        "  border-radius: 4px;"
        "  background-color: %4;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: %10;"
        "  border: %20px solid %10;"
        "}"
        "QCheckBox::indicator:disabled, QRadioButton::indicator:disabled {"
        "  background-color: %16;"
        "  border-color: %16;"
        "}"
        "QRadioButton::indicator {"
        "  border-radius: 8px;"
        "}"
        "QRadioButton::indicator:checked {"
        "  background-color: %10;"
        "  border: %20px solid %10;"
        "}"
        "QTableView, QTreeView, QListView {"
        "  background-color: %1;"
        "  alternate-background-color: %2;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-radius: 6px;"
        "  gridline-color: %8;"
        "  outline: none;"
        "}"
        "QTableView::item:selected, QTreeView::item:selected, QListView::item:selected {"
        "  background-color: %10;"
        "  color: %7;"
        "}"
        "QTableView::item:selected:disabled, QTreeView::item:selected:disabled, QListView::item:selected:disabled {"
        "  background-color: %16;"
        "  color: %17;"
        "}"
        "QHeaderView::section {"
        "  background-color: %2;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-bottom: %20px solid %8;"
        "  padding: 6px 8px;"
        "  font-weight: bold;"
        "}"
        "QHeaderView::section:selected {"
        "  background-color: %10;"
        "  color: %7;"
        "  border-color: %10;"
        "}"
        "QProgressBar {"
        "  border: %20px solid %8;"
        "  border-radius: 7px;"
        "  background-color: %4;"
        "  color: %5;"
        "  text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: %10;"
        "  border-radius: 6px;"
        "  margin: 1px;"
        "}"
        "QToolBar {"
        "  background-color: %2;"
        "  color: %5;"
        "  border: %20px solid %8;"
        "  border-radius: 6px;"
        "  spacing: 4px;"
        "}"
        "QToolButton {"
        "  background-color: transparent;"
        "  color: %5;"
        "  border: %20px solid transparent;"
        "  border-radius: 4px;"
        "  padding: 5px;"
        "}"
        "QToolButton:hover {"
        "  background-color: %3;"
        "  border: %20px solid %8;"
        "}"
        "QToolButton:pressed {"
        "  background-color: %12;"
        "}"
        "QToolButton:checked {"
        "  background-color: %10;"
        "  color: %7;"
        "}"
    )
        .arg(c.window.name())
        .arg(c.panel.name())
        .arg(c.panelHover.name())
        .arg(c.input.name())
        .arg(c.text.name())
        .arg(c.textMuted.name())
        .arg(c.textInverted.name())
        .arg(c.border.name())
        .arg(c.borderFocus.name())
        .arg(c.primary.name())
        .arg(c.primaryHover.name())
        .arg(c.primaryActive.name())
        .arg(c.success.name())
        .arg(c.warning.name())
        .arg(c.error.name())
        .arg(c.disabledBg.name())
        .arg(c.disabledText.name())
        .arg(c.tooltipBg.name())
        .arg(c.tooltipText.name())
        .arg(c.borderWidth)
        .arg(c.focusWidth)
        .arg(bgWidgetBg);
}

ThemeManager::ThemeColors ThemeManager::paletteLight() const
{
    ThemeColors c;
    c.window = QColor(0xFF, 0xFF, 0xFF);
    c.panel = QColor(0xF4, 0xF4, 0xF8);
    c.panelHover = QColor(0xE9, 0xE9, 0xEF);
    c.input = QColor(0xFF, 0xFF, 0xFF);
    c.text = QColor(0x1C, 0x1C, 0x21);
    c.textMuted = QColor(0x6E, 0x6E, 0x75);
    c.textInverted = QColor(0xFF, 0xFF, 0xFF);
    c.border = QColor(0xC9, 0xC9, 0xD1);
    c.borderFocus = QColor(0x25, 0x63, 0xEB);
    c.primary = QColor(0x25, 0x63, 0xEB);
    c.primaryHover = QColor(0x1D, 0x4E, 0xD8);
    c.primaryActive = QColor(0x1E, 0x40, 0xAF);
    c.success = QColor(0x15, 0x80, 0x3D);
    c.warning = QColor(0xB4, 0x53, 0x09);
    c.error = QColor(0xDC, 0x26, 0x26);
    c.disabledBg = QColor(0xEC, 0xEC, 0xEF);
    c.disabledText = QColor(0xA1, 0xA1, 0xAA);
    c.tooltipBg = QColor(0x1C, 0x1C, 0x21);
    c.tooltipText = QColor(0xFF, 0xFF, 0xFF);
    c.borderWidth = 1;
    c.focusWidth = 2;
    return c;
}

ThemeManager::ThemeColors ThemeManager::paletteDark() const
{
    ThemeColors c;
    c.window = QColor(0x23, 0x23, 0x29);
    c.panel = QColor(0x2D, 0x2D, 0x36);
    c.panelHover = QColor(0x38, 0x38, 0x42);
    c.input = QColor(0x3A, 0x3A, 0x44);
    c.text = QColor(0xE8, 0xE8, 0xEA);
    c.textMuted = QColor(0xA9, 0xA9, 0xB0);
    c.textInverted = QColor(0xFF, 0xFF, 0xFF);
    c.border = QColor(0x4A, 0x4A, 0x55);
    c.borderFocus = QColor(0x4A, 0x90, 0xE2);
    c.primary = QColor(0x3B, 0x8E, 0xDF);
    c.primaryHover = QColor(0x4F, 0xA3, 0xF0);
    c.primaryActive = QColor(0x2F, 0x7F, 0xD2);
    c.success = QColor(0x3F, 0xB9, 0x5F);
    c.warning = QColor(0xD9, 0xA0, 0x2B);
    c.error = QColor(0xFF, 0x6B, 0x6B);
    c.disabledBg = QColor(0x3A, 0x3A, 0x44);
    c.disabledText = QColor(0x7A, 0x7A, 0x83);
    c.tooltipBg = QColor(0x3A, 0x3A, 0x44);
    c.tooltipText = QColor(0xFF, 0xFF, 0xFF);
    c.borderWidth = 1;
    c.focusWidth = 2;
    return c;
}

ThemeManager::ThemeColors ThemeManager::paletteHighContrast() const
{
    ThemeColors c;
    c.window = QColor(0x00, 0x00, 0x00);
    c.panel = QColor(0x00, 0x00, 0x00);
    c.panelHover = QColor(0x1A, 0x1A, 0x1A);
    c.input = QColor(0x00, 0x00, 0x00);
    c.text = QColor(0xFF, 0xFF, 0xFF);
    c.textMuted = QColor(0xFF, 0xFF, 0xFF);
    c.textInverted = QColor(0x00, 0x00, 0x00);
    c.border = QColor(0xFF, 0xFF, 0xFF);
    c.borderFocus = QColor(0xFF, 0xFF, 0x00);
    c.primary = QColor(0xFF, 0xFF, 0x00);
    c.primaryHover = QColor(0xFF, 0xFF, 0x00);
    c.primaryActive = QColor(0xFF, 0xFF, 0x00);
    c.success = QColor(0x00, 0xFF, 0x00);
    c.warning = QColor(0xFF, 0xFF, 0x00);
    c.error = QColor(0xFF, 0x00, 0x00);
    c.disabledBg = QColor(0x00, 0x00, 0x00);
    c.disabledText = QColor(0xFF, 0xFF, 0xFF);
    c.tooltipBg = QColor(0x00, 0x00, 0x00);
    c.tooltipText = QColor(0xFF, 0xFF, 0xFF);
    c.borderWidth = 2;
    c.focusWidth = 3;
    return c;
}

ThemeManager::ThemeColors ThemeManager::paletteOcean() const
{
    ThemeColors c = paletteLight();
    c.panel = QColor(0xEE, 0xF4, 0xFA);
    c.panelHover = QColor(0xE2, 0xEA, 0xF2);
    c.border = QColor(0xB8, 0xCC, 0xE0);
    c.borderFocus = QColor(0x00, 0x77, 0xB6);
    c.primary = QColor(0x00, 0x77, 0xB6);
    c.primaryHover = QColor(0x00, 0x5F, 0x8E);
    c.primaryActive = QColor(0x00, 0x4C, 0x73);
    c.textMuted = QColor(0x4A, 0x6A, 0x85);
    c.success = QColor(0x1E, 0x7A, 0x46);
    c.warning = QColor(0xB4, 0x53, 0x09);
    c.error = QColor(0xC0, 0x39, 0x2B);
    return c;
}

ThemeManager::ThemeColors ThemeManager::paletteForest() const
{
    ThemeColors c = paletteLight();
    c.panel = QColor(0xEF, 0xF5, 0xEF);
    c.panelHover = QColor(0xE3, 0xEA, 0xE3);
    c.border = QColor(0xB7, 0xD3, 0xB7);
    c.borderFocus = QColor(0x2E, 0x7D, 0x32);
    c.primary = QColor(0x2E, 0x7D, 0x32);
    c.primaryHover = QColor(0x24, 0x60, 0x24);
    c.primaryActive = QColor(0x1C, 0x4E, 0x1C);
    c.textMuted = QColor(0x4E, 0x6B, 0x4E);
    c.success = QColor(0x1E, 0x7A, 0x46);
    c.warning = QColor(0xB4, 0x53, 0x09);
    c.error = QColor(0xC0, 0x39, 0x2B);
    return c;
}

ThemeManager::ThemeColors ThemeManager::paletteSunset() const
{
    ThemeColors c = paletteLight();
    c.panel = QColor(0xFB, 0xF3, 0xEC);
    c.panelHover = QColor(0xF1, 0xE4, 0xD8);
    c.border = QColor(0xE8, 0xD8, 0xC4);
    c.borderFocus = QColor(0xC7, 0x6B, 0x1A);
    c.primary = QColor(0xC7, 0x6B, 0x1A);
    c.primaryHover = QColor(0xB0, 0x5A, 0x15);
    c.primaryActive = QColor(0x8E, 0x48, 0x11);
    c.textMuted = QColor(0x8A, 0x5A, 0x2E);
    c.success = QColor(0x1E, 0x7A, 0x46);
    c.warning = QColor(0xC7, 0x6B, 0x1A);
    c.error = QColor(0xC0, 0x39, 0x2B);
    return c;
}
