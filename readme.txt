[このプログラムについて]
RYZENで起こるとされるSEGV問題を再現しようとしているコード（少なくとも自分の考えでは）

[何を行っているか]
これは"cross-modifying code"とよばれている処理です。
スレッドを3つ実行し、2つ(threadx)がメモリ上のコード領域を書き換え（コードをランダムに移動させている）、
残り1つのスレッド(thread1)がその書き換えたコードを実行し続けるということを行っています。
それぞれのスレッドはマルチスレッド同期（atomic_exchange）とメモリバリア(mfence)によって保護されていて、
書き換えと実行が混ざって行われることはありません。
また書き換えた命令を実行する直前にthread1でserializing instructionとよばれる命令(CPUID)を実行し、
命令キャッシュやパイプラインを破棄して新しい命令列が実行されることを保証しています（※）。

※これらについては以下を参照

"8.1.3 Handling Self- and Cross-Modifying Code" of
	https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html (Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 3A: System Programming Guide, Part 1)


[使い方]
Linux(Ubuntu 17.04)とWindows10(1703)上のVS2017で動作を確認しています。

Linux:
gitでcloneするなどしてコードをダウンロードしたあと以下を実行
---
# make
# ./run.sh [n] [m]
---
n: 同時実行数 (例えば8)
m: ループ数 (例えば2500000)

Windows:
ryzen_segv_testとlancherをVS2017でビルドする。
ryzen_segv_test.exeとlancher.exeを同じディレクトリに置き、lancher.exeを実行する。
concurrency(同時実行数)とloop count(ループ回数)をコマンドプロンプト画面に入力します。

ログファイルは同じディレクトリのlog.txtに出力されます。


[作成者]
Twitter: homuh0mu

------------------------------------------------------------------

[About]

This is a test code to reproduce SEGV of RYZEN processor.


[Overview]

This is a "cross-modifying code".
There a three threads (thread1 and two threadx).
Thread1 executes code of specific address repeatedly, at the same time threadx modify data of this address.
The execution and modifing are protected with multithread synchronization and memory barrier.
In addition to this, a serializing instruction is inserted before call of modified function code.

About serializing instruction in "cross-modifying code":
	"8.1.3 Handling Self- and Cross-Modifying Code" of
	https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html (Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 3A: System Programming Guide, Part 1)


[Usage]

It works with gcc on Linux (Confirmed: Ubuntu 17.04) and VS2017 on Windows10(1703).

In Linux:
---
# make
# ./run.sh [n] [m]
---
n: Number of concurrent execution (e.g. 8)
m: Loop count (e.g. 2500000)

In Windows:
Build ryzen_segv_test and lancher with VS2017.
Place ryzen_segv_test.exe and lancher.exe in same directory, and open lancher.exe.
Input concurrency and loop count to command prompt.

Output log is in log.txt.


[About me]
Twitter: homuh0mu