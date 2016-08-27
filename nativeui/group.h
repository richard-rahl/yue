// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GROUP_H_
#define NATIVEUI_GROUP_H_

#include <string>

#include "nativeui/container.h"

namespace nu {

NATIVEUI_EXPORT class Group : public View {
 public:
  Group();

  void SetContentView(Container* view);
  Container* GetContentView() const;

  void SetTitle(const std::string& title);
  std::string GetTitle() const;

 protected:
  ~Group() override;

  // Following platform implementations should only be called by wrappers.
  void PlatformInit();
  void PlatformSetContentView(Container* container);

 private:
  scoped_refptr<Container> content_view_;
};

}  // namespace nu

#endif  // NATIVEUI_GROUP_H_