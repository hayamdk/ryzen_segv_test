# このプログラムについて
RYZENで起こるとされるSEGV問題を再現しようとしているコード（少なくとも自分の考えでは）

# 何を行っているか
これは"cross-modifying code"とよばれている処理です。
スレッドを3つ実行し、2つ(threadx)がメモリ上のコード領域を書き換え（コードをランダムに移動させている）、
残り1つのスレッド(thread1)がその書き換えたコードを実行し続けるということを行っています。  
それぞれのスレッドはマルチスレッド同期（atomic_exchange）とメモリバリア(mfence)によって保護されていて、
書き換えと実行が混ざって行われることはありません。
また書き換えた命令を実行する直前にthread1でserializing instructionとよばれる命令(CPUID)を実行し、
命令キャッシュやパイプラインを破棄して新しい命令列が実行されることを保証しています（※）。

※これらについては以下を参照

* "8.1.3 Handling Self- and Cross-Modifying Code" of
	<https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html> (Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 3A: System Programming Guide, Part 1)


# 使い方
Linux(Ubuntu 17.04/16.04)とWindows10(1703)上のVS2017で動作を確認しています。  
ログファイルは同じディレクトリのlog.txtに出力されます。

## Linux
gitでcloneするなどしてコードをダウンロードしたあと以下を実行

### .cファイルからコンパイル
Ubuntu17.04 + gcc 6.3 向け

    # make asm
    # make
    # ./run.sh [n] [m]

### Ubuntu17.04上でコンパイル済みのアセンブリ(.sファイル)を使用
その他のLinux環境(Ubuntu16.04 + gcc5.4 + Kernel4.10で動作確認済み)

    # make
    # ./run.sh [n] [m]

n: 同時実行数 (例えば8)  
m: ループ数 (例えば2500000)

## Windows
ryzen_segv_testとlancherをVS2017でビルドする。
ryzen_segv_test.exeとlancher.exeを同じディレクトリに置き、lancher.exeを実行する。
concurrency(同時実行数)とloop count(ループ回数)をコマンドプロンプト画面に入力します。


# FAQ
## Q: マルチスレッド同期は正しく行われているのか？
A: atomic_exchangeでスピンロックを行っています。またthreadx側は命令列の書き換え後ロック開放前にmfenceを実行してその前後でのデータ反映の順序も保証しています。

## Q: 動的な自己書き換えはCPU的に動作を保証されないのではないのか？また割り込みなど感知できないユーザー権限のプロセスだけで行うことができるのか？
A: このコードのような処理はAMDやIntelのドキュメントでは"cross-modifying code"として言及されていて、どのようにすれば正しく動作するか示されており、そのとおりに実装したものがこちらのコードです。権限についてはドキュメント上でserializing instructionの例として特権命令と非特権命令のどちらも例として挙げられているので、非特権プロセスからもこのコードのようなことを行うことが認められていることのではないでしょうか？また割り込みなどで途中で実行するコアが切り替わった場合、OSが適切なハンドリングを行うものと考えています。カーネルからユーザーへ制御が戻る際にはiret命令が用いられるかと思いますが、例えばこのiretもserializing instructionの一例なので、カーネルから制御が戻った時点で正しくserializingが行われているのではないでしょうか？間違っていたらぜひ指摘をしていただきたいです。

## Q: RYZENのSEGVはgccなどでのビルドで起こるとされているが、このコードはそれと関係あるのか？
A: 私は関係あると考えていますが、現時点では何も断言できないです。単にこのコードを使えばRYZENでは簡単にSEGVを引き起こせるが、同じようにやっても他のプロセッサでは起こらないか起こるとしても稀であるというだけのことしか言えません。これは私の仮説ですが、RYZENのSEGVは命令キャッシュ（uopcacheやL1I）に起因する問題ではないかと当初から言われていましたが、その命令キャッシュを高頻度に汚染すれば実行結果が矛盾する瞬間があるのではないか？と睨んで行き着いたのがこのコードです。命令キャッシュを高頻度に汚染するために自己書き換えを行っているということです。繰り返しますが、このコードが絶対的に正しいこと、あるいは正しいとしてgccなどで起こるSEGVと同じ現象かというのは現時点では断言できないです。

## Q: Intelマシン(3770K)でもSEGVやmismatchが起こるらしいが
A: そうですね。可能性としては色々考えられると思います。(1)このコードのコンセプトに不備がある、(2)ハードの不調、(3)OSやコンパイラのバグ、(4)3770K等に未修正のバグがある、どれも否定はしきれないです（普通に考えて(1)じゃないの？と言われるでしょう）。ただ、私の所有するIntelマシンではどれもそのようなエラーは起こせていないのでなかなか詳しい調査はできずにいます（近いうちに私の環境の情報は整理してまとめます）。指摘、他の環境での報告は大歓迎ですので、よろしくお願いします。


# 作成者
Twitter: homuh0mu

----

# About

This is a test code to reproduce SEGV of RYZEN processor (At least in my thought).


# Overview

This is a "cross-modifying code".
There a three threads (thread1 and two threadx).
Thread1 executes code of specific address repeatedly, at the same time threadx modify data of this address.
The execution and modifing are protected with multithread synchronization and memory barrier.
In addition to this, a serializing instruction is inserted before call of modified function code.

* About serializing instruction in "cross-modifying code":  
	"8.1.3 Handling Self- and Cross-Modifying Code" of
	<https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html> (Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 3A: System Programming Guide, Part 1)


# Usage

It works with gcc on Linux (Confirmed: Ubuntu 17.04/16.04) and VS2017 on Windows10(1703).  
Output log is in log.txt.

### Compile from C source code
For Ubuntu 17.04 + gcc 6.3

    # make asm
    # make
    # ./run.sh [n] [m]

### Use compiled assemblies which was generated on Ubuntu17.04
For other Linux (Confirmed: Ubuntu 16.04 + gcc 5.4 + Kernel 4.10)

    # make
    # ./run.sh [n] [m]

n: Number of concurrent execution (e.g. 8)  
m: Loop count (e.g. 2500000)

## Windows
Build ryzen_segv_test and lancher with VS2017.
Place ryzen_segv_test.exe and lancher.exe in same directory, and open lancher.exe.
Input concurrency and loop count to command prompt.


# Author
Twitter: homuh0mu