# Exact-version compatibility / 精確版本相容

The three-profile architecture introduced in 0.4.1 is retained in 0.5.0-test1.
Each profile contains its own image identities, whole-file and `.text` SHA-256,
entry guards, native addresses and layout. Unknown address keys return zero.
Loaded headers are compared separately (normalizing only ASLR ImageBase), and
live hook entry/owner checks are mandatory. There is no runtime scanner or
uniform address-delta fallback.

1.26.45.01 has independently reviewed scene-stack, view/controller, coordinate
and locale layouts. Its wheel/text targets are outer scene thunks, not inner
view methods. 1.26.51.01 admits only the exact reviewed GUI/console launcher
forms through separate whole-file hashes; arbitrary PE changes are refused.
Original and launcher forms of 1.26.52.03 are also admitted individually.

The Mouse Tweaks bridge preserves the original caller return address and accepts
only recognized DLL identity, ownership and Ready-state contracts. An old or
unknown shared-hook owner is not grounds to weaken those checks. Do not replace
another plugin simply because its basename is the same.

Only audited vanilla JSON-UI menu structures are transformed. Unrecognized,
ambiguous or conflicting custom layouts are left untouched. These checks do not
establish OreUI or arbitrary resource-pack compatibility.

0.5.0-test1 沿用獨立的三版本配置：完整檔案／程式碼雜湊、入口、位址及結構布局均精確
比對；未知位址回傳零，不以其他版本差值補齊。26.45 的布局與外層轉發入口獨立核實；
26.51 僅允許精確的 GUI／主控台啟動器映像，不能推廣到任意修改過的 EXE。
Mouse Tweaks 橋接保留原呼叫返回位址，仍驗證精確 DLL、持有者與 Ready 狀態。
選單只轉換已核實結構，不能辨識或有衝突的自訂布局保持原樣。
