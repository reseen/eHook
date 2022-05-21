# eHook

本工具能使 Windows Emacs 与 Windows 其他应用的 CUA 操作保持一致，比如在 vscode 下面执行 Ctrl+C ，然后到 Emacs 中执行 Ctrl+V，小拇指不移动。

---

这两天看子龙山人老师的 2022 版 《21天学会 Emacs》，第一节改键就讲了他把 Left Window 键改成 Left Alt 键，Left Alt 键改成 App 键，大小写键改成 Ctrl 键的方案，兼容了 Windows 和 Mac 操作的一致性。这个改 Super 键的思路，给了我灵感。如果将左Ctrl键改为Super，CapsLock 键改为左 Ctrl 键，并且仅在 Emacs 窗口激活时生效，再配合子龙山人的改键配置，即可实现目标。

使用方法：
参考2022版 《21天学会 Emacs》第一节教程，使用本工具，代替 ShapsKey 改键，使改键方案只在 Emacs 窗口获得焦点时启用。

Emacs 改键如下：

```
(setq w32-apps-modifier 'super)                 ; 通过SharpKeys改成了 Application
(global-set-key (kbd "s-a") 'mark-whole-buffer) ; 对应Windows上面的Ctrl-a 全选
(global-set-key (kbd "s-c") 'kill-ring-save)    ; 对应Windows上面的Ctrl-c 复制
(global-set-key (kbd "s-s") 'save-buffer)       ; 对应Windows上面的Ctrl-s 保存
(global-set-key (kbd "s-v") 'yank)              ; 对应Windows上面的Ctrl-v 粘贴
(global-set-key (kbd "s-z") 'undo)              ; 对应Windows上面的Ctrol-z 撤销
(global-set-key (kbd "s-x") 'kill-region)       ; 对应Windows上面的Ctrol-x 剪
```

最终键值定义：

```
CapsLock -> LeftCtrl
LeftCtrl -> Super
RightCtrl -> CapsLock
```
