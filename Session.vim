let SessionLoad = 1
let s:so_save = &g:so | let s:siso_save = &g:siso | setg so=0 siso=0 | setl so=-1 siso=-1
let v:this_session=expand("<sfile>:p")
silent only
silent tabonly
cd ~/Projects/wholth
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
let s:shortmess_save = &shortmess
if &shortmess =~ 'A'
  set shortmess=aoOA
else
  set shortmess=aoO
endif
badd +231 ./src/main_2.cpp
badd +3730 C:/VulkanSDK/1.3.250.0/Include/vulkan/vulkan_core.h
argglobal
%argdel
$argadd ./src/main_2.cpp
edit ./src/main_2.cpp
argglobal
balt C:/VulkanSDK/1.3.250.0/Include/vulkan/vulkan_core.h
setlocal fdm=indent
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=8
setlocal fml=1
setlocal fdn=20
setlocal fen
23
normal! zo
115
normal! zo
142
normal! zo
178
normal! zo
188
normal! zo
194
normal! zo
199
normal! zo
199
normal! zo
226
normal! zo
230
normal! zo
236
normal! zo
278
normal! zo
289
normal! zo
305
normal! zo
317
normal! zo
319
normal! zo
let s:l = 236 - ((28 * winheight(0) + 23) / 46)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 236
normal! 052|
tabnext 1
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0 && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20
let &shortmess = s:shortmess_save
let s:sx = expand("<sfile>:p:r")."x.vim"
if filereadable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &g:so = s:so_save | let &g:siso = s:siso_save
set hlsearch
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
