// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include <richedit.h>

#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/edit_view.h"

namespace nu {

namespace {

class TextEditImpl : public EditView {
 public:
  explicit TextEditImpl(View* delegate)
      : EditView(delegate, WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL) {
    set_switch_focus_on_tab(false);
    SetPlainText();
  }

  Rect request_size() const { return request_size_; }

 protected:
  // SubwinView:
  void OnCommand(UINT code, int command) override {
    TextEdit* edit = static_cast<TextEdit*>(delegate());
    if (code == EN_CHANGE)
      edit->on_text_change.Emit(edit);
  }

  LRESULT OnNotify(int id, LPNMHDR pnmh) override {
    if (pnmh->code == EN_REQUESTRESIZE)
      request_size_ = Rect(reinterpret_cast<REQRESIZE*>(pnmh)->rc);
    return 0;
  }

 private:
  CR_BEGIN_MSG_MAP_EX(TextEditImpl, SubwinView)
    CR_MSG_WM_KEYDOWN(OnKeyDown)
  CR_END_MSG_MAP()

  void OnKeyDown(UINT ch, UINT repeat, UINT flags) {
    TextEdit* edit = static_cast<TextEdit*>(delegate());
    if (ch == VK_RETURN && edit->should_insert_new_line)
      SetMsgHandled(!edit->should_insert_new_line(edit));
    else
      SetMsgHandled(false);
  }

 private:
  Rect request_size_;
};

}  // namespace

TextEdit::TextEdit() {
  TakeOverView(new TextEditImpl(this));
}

TextEdit::~TextEdit() {
}

void TextEdit::SetText(const std::string& text) {
  static_cast<EditView*>(GetNative())->SetText(text);
}

std::string TextEdit::GetText() const {
  return static_cast<EditView*>(GetNative())->GetText();
}

void TextEdit::Redo() {
  static_cast<EditView*>(GetNative())->Redo();
}

bool TextEdit::CanRedo() const {
  return static_cast<EditView*>(GetNative())->CanRedo();
}

void TextEdit::Undo() {
  static_cast<EditView*>(GetNative())->Undo();
}

bool TextEdit::CanUndo() const {
  return static_cast<EditView*>(GetNative())->CanUndo();
}

void TextEdit::Cut() {
  static_cast<EditView*>(GetNative())->Cut();
}

void TextEdit::Copy() {
  static_cast<EditView*>(GetNative())->Copy();
}

void TextEdit::Paste() {
  static_cast<EditView*>(GetNative())->Paste();
}

void TextEdit::SelectAll() {
  static_cast<EditView*>(GetNative())->SelectAll();
}

std::tuple<int, int> TextEdit::GetSelectionRange() const {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  int start, end;
  ::SendMessage(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&start),
                                 reinterpret_cast<LPARAM>(&end));
  if (start == end)
    return std::make_tuple(-1, -1);
  else
    return std::make_tuple(start, end);
}

void TextEdit::SelectRange(int start, int end) {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, EM_SETSEL, start, end);
  ::SendMessage(hwnd, EM_SCROLLCARET, 0, 0L);
}

std::string TextEdit::GetTextInRange(int start, int end) const {
  return GetText().substr(start, end - start);
}

void TextEdit::InsertText(const std::string& text) {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessageW(hwnd, EM_REPLACESEL, TRUE,
                 reinterpret_cast<LPARAM>(base::UTF8ToUTF16(text).c_str()));
}

void TextEdit::InsertTextAt(const std::string& text, int pos) {
  SelectRange(pos, pos);
  InsertText(text);
}

void TextEdit::Delete() {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  ::SendMessage(hwnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(L""));
}

void TextEdit::DeleteRange(int start, int end) {
  SelectRange(start, end);
  InsertText("");
}

void TextEdit::SetScrollbarPolicy(Scroll::Policy h_policy,
                                  Scroll::Policy v_policy) {
  HWND hwnd = static_cast<SubwinView*>(GetNative())->hwnd();
  DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
  style = style & (~WS_VSCROLL) & (~WS_HSCROLL);
  if (h_policy != Scroll::Policy::Never)
    style |= WS_HSCROLL;
  if (v_policy != Scroll::Policy::Never)
    style |= WS_VSCROLL;
  if (h_policy == Scroll::Policy::Always &&
      v_policy == Scroll::Policy::Always)
    style |= ES_DISABLENOSCROLL;
  ::SetWindowLong(hwnd, GWL_STYLE, style);
}

RectF TextEdit::GetTextBounds() const {
  auto* edit = static_cast<TextEditImpl*>(GetNative());
  ::SendMessage(edit->hwnd(), EM_REQUESTRESIZE, 0, 0L);
  return ScaleRect(RectF(edit->request_size()), 1.0f / edit->scale_factor());
}

}  // namespace nu
