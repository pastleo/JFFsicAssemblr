Just For Fun SIC Assembler
===

別認真，這個只是系上的期末作業，我雖然一開始很想要寫一個 SIC/XE 的組譯器(或者是說有延伸功能)，最後因為各種期末，還是只有這個鬼樣子，就當作一次大型 C++ 練習吧


## 特色

 * 使用 Hash Table
 * 原始碼單行可以有多個指令(後來覺得蠻無聊的功能)

## 使用方法

 * 你必須要自己編譯，因為這個組譯器執行檔是用 CentOS 或者 OSX 編譯的

        g++ sicAssemblr.cpp a.out

 * 然後你就可以來玩玩看

        ./a.out test1.sic [-d] [destination_file] [intermediate_file]
        // add -d to dump the hash table at the end

