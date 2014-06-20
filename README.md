Just For Fun SIC Assembler
===

別認真，這個只是系上的期末作業，我雖然一開始很想要寫一個 SIC/XE 的組譯器(或者是說有延伸功能)，最後因為各種期末，還是只有這個鬼樣子，就當作一次大型 C++ 練習吧

注意本 Repo 沒有附上已經編譯好之執行檔，各位必須要用`g++`或是其他編譯器自行編譯

# ReadMe

## Sic Assembler
A simple sic assembler

 * Author: PastLeo
 * Email: chgu82837@gmail.com
 * starting date: 2014/06/1
 * finishing date: 2014/06/15

## 特色

 * 使用傳入 callback 的 Hash Table
 * 原始碼單行可以有多個指令

## 使用方法

     the_exe [-d] [src_file] [output_file] [intermediate_file]

 * the_exe is the execution file, it might be `a.exe` or `a.out`.
 * option [-d]: After all process done,dump hash table.
 * if no src_file name specified, will prompt user to enter one.
 * if no output_file name specified, will use `a.des` as output_file name.
 * if no intermediate_file specified, will use `intermediate` as intermediate_file name.
