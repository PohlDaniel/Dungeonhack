/*!
	@file
	@author		Albert Semenov
	@date		11/2007
	@module
*//*
	This file is part of MyGUI.
	
	MyGUI is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	MyGUI is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with MyGUI.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MYGUI_EDIT_H__
#define __MYGUI_EDIT_H__

#include "MyGUI_Prerequest.h"
#include "MyGUI_StaticText.h"
#include "MyGUI_TextChangeHistory.h"
#include "MyGUI_TextIterator.h"
#include "MyGUI_EventPair.h"
#include "MyGUI_ScrollViewBase.h"

namespace MyGUI
{

	typedef delegates::CDelegate1<EditPtr> EventHandle_EditPtr;

	class MYGUI_EXPORT Edit :
		public StaticText,
		public ScrollViewBase
	{
		// для вызова закрытого конструктора
		friend class factory::BaseWidgetFactory<Edit>;

		MYGUI_RTTI_CHILD_HEADER( Edit, StaticText );

	public:
		/** Colour interval */
		void setTextIntervalColour(size_t _start, size_t _count, const Colour& _colour);

		/** Get index of first selected character or ITEM_NONE if nothing selected */
		size_t getTextSelectionStart();

		/** Get index of last selected character or ITEM_NONE if nothing selected */
		size_t getTextSelectionEnd();

		// возвращает длинну выделения !!! ПРОВЕРИТЬ
		/** Get length of selected text */
		size_t getTextSelectionLength();

		// возвращает текст с тегами
		/** Get _count characters with tags from _start position */
		Ogre::UTFString getTextInterval(size_t _start, size_t _count);

		/** Set selected text interval
			@param _start of interval
			@param _end of interval
		*/
		void setTextSelection(size_t _start, size_t _end);

		/** Delete selected text */
		void deleteTextSelection();

		/** Get selected text */
		Ogre::UTFString getTextSelection();

		/** Is any text selected */
		bool isTextSelection();


		/** Colour selected text */
		void setTextSelectionColour(const Colour& _colour);


		/** Set text cursor position */
		void setTextCursor(size_t index);
		/** Get text cursor position */
		size_t getTextCursor() { return mCursorPosition; }


		/** Set edit text applying tags */
		virtual void setCaption(const Ogre::UTFString & _caption);
		/** Get edit text with tags */
		virtual const Ogre::UTFString& getCaption();

		/** Set edit text without tags */
		void setOnlyText(const Ogre::UTFString & _text);
		/** Get edit text without tags */
		Ogre::UTFString getOnlyText();

		/** Get text length excluding tags
			For example "#00FF00Hello" length is 5
		*/
		size_t getTextLength() { return mTextLength; }

		//! Sets if surplus characters should push characters off the left side rather than ignored
		void setOverflowToTheLeft(bool _overflowToLeft) { mOverflowToTheLeft = _overflowToLeft; }

		//! Returns true if surplus characters will be pushed off the left rather than ignored
		bool getOverflowToTheLeft() { return mOverflowToTheLeft; }

		//! Sets the max amount of text allowed in the edit field.
		void setMaxTextLength(size_t _maxTextLength) { mMaxTextLength = _maxTextLength; }

		//! Gets the max amount of text allowed in the edit field.
		size_t getMaxTextLength() { return mMaxTextLength; }

		// вставляет текст в указанную позицию
		/** Inser text at _index position (text end by default) */
		void insertText(const Ogre::UTFString & _text, size_t _index = ITEM_NONE);
		// добавляет текст в конец
		/** Add text */
		void addText(const Ogre::UTFString & _text);
		/** Erase _count characters from _start position */
		void eraseText(size_t _start, size_t _count = 1);

		/** Enable or disable edit read only mode
			Read only mode: you can't edit text, but can select it.\n
			Disabled (false) by default.
		*/
		void setEditReadOnly(bool _read);
		/** Get edit read only mode flag */
		bool getEditReadOnly() { return mModeReadOnly; }

		/** Enable or disable edit password mode
			Password mode: you see password chars (*** by default) instead text.\n
			Disabled (false) by default.
		*/
		void setEditPassword(bool _password);
		/** Get edit password mode flag */
		bool getEditPassword() { return mModePassword; }

		/** Enable or disable edit multiline mode
			Multile mode: new line character moves text to new line.
			Otherwise new lines replaced with space and all text is in single line.\n
			Disabled (false) by default.
		*/
		void setEditMultiLine(bool _multi);
		/** Get edit multiline mode flag */
		bool getEditMultiLine() { return mModeMultiline; }

		/** Enable or disable edit static mode */

		/** Enable or disable edit static mode
			Static mode is same as read only, but you also can't select text.\n
			Disabled (false) by default.
		*/
		void setEditStatic(bool _static);

		/** Get edit static mode flag */
		bool getEditStatic() { return mModeStatic; }

		/** Set edit password character ('*' by default) */
		void setPasswordChar(Char _char);
		/** Set edit password character ('*' by default). First character of string used. */
		void setPasswordChar(const Ogre::UTFString & _char);
		/** Get edit password character */
		Char getPasswordChar() { return mCharPassword; }

		/** Enable or disable edit word wrap mode
			Word Wrap mode: move words to new line if they goes out of width.
			Also in this mode you can't edit or select text.\n
			Disabled (false) by default.
		*/
		void setEditWordWrap(bool _wordwrap);
		/** Get edit word wrap mode flag */
		bool getEditWordWrap() { return mModeWordWrap; }

		/** Enable or disable tab printing mode
			Tab printing mode: when editing text and pressing Tab key it displayed.
			If this mode disabled Tab key ignored.\n
			Disabled (false) by default.
		*/
		void setTabPrinting(bool _print) { mTabPrinting = _print; }
		/** Get edit tab printing wrap mode flag */
		bool getTabPrinting() { return mTabPrinting; }


		//! @copydoc Widget::setPosition(const IntPoint & _point)
		virtual void setPosition(const IntPoint & _point);
		//! @copydoc Widget::setSize(const IntSize& _size)
		virtual void setSize(const IntSize & _size);
		//! @copydoc Widget::setCoord(const IntCoord & _coord)
		virtual void setCoord(const IntCoord & _coord);

		/** @copydoc Widget::setPosition(int _left, int _top) */
		void setPosition(int _left, int _top) { setPosition(IntPoint(_left, _top)); }
		/** @copydoc Widget::setSize(int _width, int _height) */
		void setSize(int _width, int _height) { setSize(IntSize(_width, _height)); }
		/** @copydoc Widget::setCoord(int _left, int _top, int _width, int _height) */
		void setCoord(int _left, int _top, int _width, int _height) { setCoord(IntCoord(_left, _top, _width, _height)); }

		/** Show VScroll when text size larger than Edit */
		void setVisibleVScroll(bool _visible);
		/** Get Show VScroll flag */
		bool isVisibleVScroll() { return mVisibleVScroll; }

		/** Show HScroll when text size larger than Edit */
		void setVisibleHScroll(bool _visible);
		/** Get Show HScroll flag */
		bool isVisibleHScroll() { return mVisibleHScroll; }

		size_t getVScrollRange();
		size_t getVScrollPosition();
		void setVScrollPosition(size_t _index);

		size_t getHScrollRange();
		size_t getHScrollPosition();
		void setHScrollPosition(size_t _index);

		//! @copydoc StaticText::setFontName
		virtual void setFontName(const std::string & _font);
		//! @copydoc StaticText::setFontHeight
		virtual void setFontHeight(uint _height);

		//! @copydoc StaticText::setTextAlign
		virtual void setTextAlign(Align _align);

	/*event:*/
		/** Event : Enter pressed (Ctrl+enter in multiline mode).\n
			signature : void method(MyGUI::EditPtr _sender)
			@param _sender widget that called this event
		*/
		EventPair<EventHandle_WidgetVoid, EventHandle_EditPtr> eventEditSelectAccept;

		/** Event : Text changed.\n
			signature : void method(MyGUI::EditPtr _sender)
			@param _sender widget that called this event
		*/
		EventPair<EventHandle_WidgetVoid, EventHandle_EditPtr> eventEditTextChange;


	/*obsolete:*/
#ifndef MYGUI_DONT_USE_OBSOLETE

		MYGUI_OBSOLETE("use : void Edit::setVisibleVScroll(bool _visible)")
		void showVScroll(bool _visible) { setVisibleVScroll(_visible); }
		MYGUI_OBSOLETE("use : bool Edit::isVisibleVScroll()")
		bool isShowVScroll() { return isVisibleVScroll(); }
		MYGUI_OBSOLETE("use : void Edit::setVisibleHScroll(bool _visible)")
		void showHScroll(bool _visible) { setVisibleHScroll(_visible); }
		MYGUI_OBSOLETE("use : bool Edit::isVisibleHScroll()")
		bool isShowHScroll() { return isVisibleHScroll(); }

		MYGUI_OBSOLETE("use : void Widget::setCoord(const IntCoord& _coord)")
		void setPosition(const IntCoord & _coord) { setCoord(_coord); }
		MYGUI_OBSOLETE("use : void Widget::setCoord(int _left, int _top, int _width, int _height)")
		void setPosition(int _left, int _top, int _width, int _height) { setCoord(_left, _top, _width, _height); }

		// to avoid hiding base virtual function
		virtual void setTextColour(const Colour& _colour){Base::setTextColour(_colour);};

		MYGUI_OBSOLETE("use : void setTextIntervalColour(size_t _start, size_t _count, const Colour& _colour)")
		void setTextColour(size_t _start, size_t _count, const Colour& _colour) { setTextIntervalColour(_start, _count, _colour); }

		MYGUI_OBSOLETE("use : size_t getTextSelectionStart() , size_t getTextSelectionEnd()")
		void getTextSelect(size_t & _start, size_t & _end);

		MYGUI_OBSOLETE("use : Ogre::UTFString getTextInterval(size_t _start, size_t _count)")
		Ogre::UTFString getText(size_t _start, size_t _count) { return getTextInterval(_start, _count); }

		MYGUI_OBSOLETE("use : void setTextSelection(size_t _start, size_t _end)")
		void setTextSelect(size_t _start, size_t _end) { setTextSelection(_start, _end); }

		MYGUI_OBSOLETE("use : void deleteTextSelection()")
		void deleteTextSelect() { deleteTextSelection(); }

		MYGUI_OBSOLETE("use : Ogre::UTFString getTextSelection()")
		Ogre::UTFString getSelectedText() { return getTextSelection(); }

		MYGUI_OBSOLETE("use : bool isTextSelection()")
		bool isTextSelect() { return isTextSelection(); }

		MYGUI_OBSOLETE("use : void setTextSelectionColour(const Colour& _colour)")
		void setTextSelectColour(const Colour& _colour) { setTextSelectionColour(_colour); }

#endif // MYGUI_DONT_USE_OBSOLETE


	protected:
		Edit(WidgetStyle _style, const IntCoord& _coord, Align _align, const WidgetSkinInfoPtr _info, WidgetPtr _parent, ICroppedRectangle * _croppedParent, IWidgetCreator * _creator, const std::string & _name);
		virtual ~Edit();

		virtual void onMouseDrag(int _left, int _top);
		virtual void onKeyLostFocus(WidgetPtr _new);
		virtual void onKeySetFocus(WidgetPtr _old);
		virtual void onKeyButtonPressed(KeyCode _key, Char _char);

		// потом убрать все нотифи в сраку
		void notifyMouseSetFocus(WidgetPtr _sender, WidgetPtr _old);
		void notifyMouseLostFocus(WidgetPtr _sender, WidgetPtr _new);
		void notifyMousePressed(WidgetPtr _sender, int _left, int _top, MouseButton _id);
		void notifyMouseReleased(WidgetPtr _sender, int _left, int _top, MouseButton _id);
		void notifyMouseDrag(WidgetPtr _sender, int _left, int _top);
		void notifyMouseButtonDoubleClick(WidgetPtr _sender);

		void notifyScrollChangePosition(VScrollPtr _sender, size_t _position);
		void notifyMouseWheel(WidgetPtr _sender, int _rel);

		// обновление представления
		void updateView();

		void baseChangeWidgetSkin(WidgetSkinInfoPtr _info);

	private:
		void initialiseWidgetSkin(WidgetSkinInfoPtr _info);
		void shutdownWidgetSkin();

	private:
		// устанавливает текст
		void setText(const Ogre::UTFString & _text, bool _history);
		// удаляет все что выделенно
		bool deleteTextSelect(bool _history);
		// вставляет текст в указанную позицию
		void insertText(const Ogre::UTFString & _text, size_t _index, bool _history);
		// удаляет текст
		void eraseText(size_t _start, size_t _count, bool _history);
		// выделяет цветом выделение
		void setTextSelectColour(const Colour& _colour, bool _history);
		// выделяет цветом диапазон
		void _setTextColour(size_t _start, size_t _count, const Colour& _colour, bool _history);

		void frameEntered(float _frame);

		void updateEditState();

		// обновляет курсор по координате
		void updateSelectText();

		void resetSelect();

		// запись в историю данных о позиции
		void commandPosition(size_t _undo, size_t _redo, size_t _length, VectorChangeInfo * _info = nullptr);

		// команнды отмена и повтор
		bool commandRedo();
		bool commandUndo();
		// объединяет последние две комманды
		void commandMerge();
		// очистка
		void commandResetRedo() {mVectorRedoChangeInfo.clear();}
		void commandResetHistory() {mVectorRedoChangeInfo.clear();mVectorUndoChangeInfo.clear();}
		void saveInHistory(VectorChangeInfo * _info = nullptr);

		// работа с буфером обмена
		void commandCut();
		void commandCopy();
		void commandPast();


		const Ogre::UTFString & getRealString();

		void setRealString(const Ogre::UTFString & _caption);

		void eraseView();
		void updateViewWithCursor();
		void updateCursorPosition();

		void setContentPosition(const IntPoint& _point);
		IntSize getViewSize();
		IntSize getContentSize();
		size_t getVScrollPage();
		size_t getHScrollPage();
		IntPoint getContentPosition();
		Align getContentAlign();

	protected:
		// нажата ли кнопка
		bool mIsPressed;
		// в фокусе ли кнопка
		bool mIsFocus;

		bool mCursorActive;
		float mCursorTimer, mActionMouseTimer;

		// позиция курсора
		size_t mCursorPosition;
		// максимальное колличество
		size_t mTextLength;

		// выделение
		size_t mStartSelect, mEndSelect;

		// списоки изменений для отмены и повтора
		DequeUndoRedoInfo mVectorUndoChangeInfo;
		DequeUndoRedoInfo mVectorRedoChangeInfo;

		bool mMouseLeftPressed;

		bool mModeReadOnly;
		bool mModePassword;
		bool mModeMultiline;
		bool mModeStatic;
		bool mModeWordWrap;

		bool mTabPrinting;

		// настоящий текст, закрытый за звездочками
		Ogre::UTFString mPasswordText;

		// для поддержки режима статик, где курсор не нужен
		std::string mOriginalPointer;

		Char mCharPassword;

		bool mOverflowToTheLeft;
		size_t mMaxTextLength;

	};

} // namespace MyGUI

#endif // __MYGUI_EDIT_H__
