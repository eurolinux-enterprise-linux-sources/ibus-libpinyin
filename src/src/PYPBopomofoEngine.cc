/* vim:set et ts=4 sts=4:
 *
 * ibus-libpinyin - Intelligent Pinyin engine based on libpinyin for IBus
 *
 * Copyright (c) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (c) 2010 BYVoid <byvoid1@gmail.com>
 * Copyright (c) 2011 Peng Wu <alexepico@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "PYPBopomofoEngine.h"
#include <string>
#include "PYPunctEditor.h"
#include "PYPBopomofoEditor.h"
#include "PYFallbackEditor.h"
#include "PYConfig.h"
#include "PYPConfig.h"

using namespace PY;

/* constructor */
LibPinyinBopomofoEngine::LibPinyinBopomofoEngine (IBusEngine *engine)
    : Engine (engine),
      m_props (LibPinyinBopomofoConfig::instance ()),
      m_prev_pressed_key (IBUS_VoidSymbol),
      m_input_mode (MODE_INIT),
      m_fallback_editor (new FallbackEditor (m_props, LibPinyinBopomofoConfig::instance()))
{
    gint i;

    /* create editors */
    m_editors[MODE_INIT].reset (new LibPinyinBopomofoEditor (m_props, LibPinyinBopomofoConfig::instance ()));
    m_editors[MODE_PUNCT].reset (new PunctEditor (m_props, LibPinyinBopomofoConfig::instance ()));

    m_props.signalUpdateProperty ().connect
        (std::bind (&LibPinyinBopomofoEngine::updateProperty, this, _1));

    for (i = MODE_INIT; i < MODE_LAST; i++) {
        connectEditorSignals (m_editors[i]);
    }

    connectEditorSignals (m_fallback_editor);
}

/* destructor */
LibPinyinBopomofoEngine::~LibPinyinBopomofoEngine (void)
{
}

gboolean
LibPinyinBopomofoEngine::processKeyEvent (guint keyval, guint keycode, guint modifiers)
{
    gboolean retval = FALSE;

    /* check Shift or Ctrl + Release hotkey,
     * and then ignore other Release key event */
    if (modifiers & IBUS_RELEASE_MASK) {
        /* press and release keyval are same,
         * and no other key event between the press and release key event */
        gboolean triggered = FALSE;

        if (m_prev_pressed_key == keyval) {
            if (LibPinyinBopomofoConfig::instance ().ctrlSwitch ()) {
                if (keyval == IBUS_Control_L || keyval == IBUS_Control_R)
                    triggered = TRUE;
            } else {
                if (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R)
                    triggered = TRUE;
            }
        }

        if (triggered) {
            if (!m_editors[MODE_INIT]->text ().empty ())
                m_editors[MODE_INIT]->reset ();
            m_props.toggleModeChinese ();
            return TRUE;
        }

        if (m_input_mode == MODE_INIT &&
            m_editors[MODE_INIT]->text ().empty ()) {
            /* If it is init mode, and no any previous input text,
             * we will let client applications to handle release key event */
            return FALSE;
        } else {
            return TRUE;
        }
    }

    /* Toggle simp/trad Chinese Mode when hotkey Ctrl + Shift + F pressed */
    if (keyval == IBUS_F && scmshm_test (modifiers, (IBUS_SHIFT_MASK | IBUS_CONTROL_MASK))) {
        m_props.toggleModeSimp();
        m_prev_pressed_key = IBUS_F;
        return TRUE;
    }

    if (m_props.modeChinese ()) {
        if (G_UNLIKELY (m_input_mode == MODE_INIT &&
                        m_editors[MODE_INIT]->text ().empty () &&
                        cmshm_filter (modifiers) == 0 &&
                        keyval == IBUS_grave)){
            /* if BopomofoEditor is empty and get a grave key,
             * switch current editor to PunctEditor */
            m_input_mode = MODE_PUNCT;
        }

        retval = m_editors[m_input_mode]->processKeyEvent (keyval, keycode, modifiers);
        if (G_UNLIKELY (retval &&
                        m_input_mode != MODE_INIT &&
                        m_editors[m_input_mode]->text ().empty ()))
            m_input_mode = MODE_INIT;
    }

    if (G_UNLIKELY (!retval))
        retval = m_fallback_editor->processKeyEvent (keyval, keycode, modifiers);

    /* store ignored key event by editors */
    m_prev_pressed_key = retval ? IBUS_VoidSymbol : keyval;

    return retval;
}

void
LibPinyinBopomofoEngine::focusIn (void)
{
    registerProperties (m_props.properties ());
}

void
LibPinyinBopomofoEngine::focusOut (void)
{
    reset ();
}

void
LibPinyinBopomofoEngine::reset (void)
{
    m_prev_pressed_key = IBUS_VoidSymbol;
    m_input_mode = MODE_INIT;
    for (gint i = 0; i < MODE_LAST; i++) {
        m_editors[i]->reset ();
    }
    m_fallback_editor->reset ();
}

void
LibPinyinBopomofoEngine::enable (void)
{
    m_props.reset ();
}

void
LibPinyinBopomofoEngine::disable (void)
{
}

void
LibPinyinBopomofoEngine::pageUp (void)
{
    m_editors[m_input_mode]->pageUp ();
}

void
LibPinyinBopomofoEngine::pageDown (void)
{
    m_editors[m_input_mode]->pageDown ();
}

void
LibPinyinBopomofoEngine::cursorUp (void)
{
    m_editors[m_input_mode]->cursorUp ();
}

void
LibPinyinBopomofoEngine::cursorDown (void)
{
    m_editors[m_input_mode]->cursorDown ();
}

inline void
LibPinyinBopomofoEngine::showSetupDialog (void)
{
    g_spawn_command_line_async
        (LIBEXECDIR"/ibus-setup-libpinyin bopomofo", NULL);
}

gboolean
LibPinyinBopomofoEngine::propertyActivate (const gchar *prop_name,
                                           guint prop_state)
{
    const static std::string setup ("setup");
    if (m_props.propertyActivate (prop_name, prop_state)) {
        return TRUE;
    }
    else if (setup == prop_name) {
        showSetupDialog ();
        return TRUE;
    }
    return FALSE;
}

void
LibPinyinBopomofoEngine::candidateClicked (guint index,
                                           guint button,
                                           guint state)
{
    m_editors[m_input_mode]->candidateClicked (index, button, state);
}

void
LibPinyinBopomofoEngine::commitText (Text & text)
{
    Engine::commitText (text);
    if (m_input_mode != MODE_INIT)
        m_input_mode = MODE_INIT;
#if 1
    /* handle "<num>+.<num>+" here */
    if (text.text ())
        static_cast<FallbackEditor*> (m_fallback_editor.get ())->setPrevCommittedChar (*text.text ());
    else
        static_cast<FallbackEditor*> (m_fallback_editor.get ())->setPrevCommittedChar (0);
#endif
}

void
LibPinyinBopomofoEngine::connectEditorSignals (EditorPtr editor)
{
    editor->signalCommitText ().connect (
        std::bind (&LibPinyinBopomofoEngine::commitText, this, _1));

    editor->signalUpdatePreeditText ().connect (
        std::bind (&LibPinyinBopomofoEngine::updatePreeditText, this, _1, _2, _3));
    editor->signalShowPreeditText ().connect (
        std::bind (&LibPinyinBopomofoEngine::showPreeditText, this));
    editor->signalHidePreeditText ().connect (
        std::bind (&LibPinyinBopomofoEngine::hidePreeditText, this));

    editor->signalUpdateAuxiliaryText ().connect (
        std::bind (&LibPinyinBopomofoEngine::updateAuxiliaryText, this, _1, _2));
    editor->signalShowAuxiliaryText ().connect (
        std::bind (&LibPinyinBopomofoEngine::showAuxiliaryText, this));
    editor->signalHideAuxiliaryText ().connect (
        std::bind (&LibPinyinBopomofoEngine::hideAuxiliaryText, this));

    editor->signalUpdateLookupTable ().connect (
        std::bind (&LibPinyinBopomofoEngine::updateLookupTable, this, _1, _2));
    editor->signalUpdateLookupTableFast ().connect (
        std::bind (&LibPinyinBopomofoEngine::updateLookupTableFast, this, _1, _2));
    editor->signalShowLookupTable ().connect (
        std::bind (&LibPinyinBopomofoEngine::showLookupTable, this));
    editor->signalHideLookupTable ().connect (
        std::bind (&LibPinyinBopomofoEngine::hideLookupTable, this));
}


