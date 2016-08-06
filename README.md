# Denver Streaming Server
<b>Requirements</b><br />
//Install lastest FFmpeg software properly first.<br />
//In this progress, you may need to install some supporting software.<br />
$./configure --enable-version3 --enable-nonfree --enable-gpl --enable-libfaac --enable-libfdk-aac --enable-libmp3lame --enable-libopenh264 --enable-libopenjpeg --enable-libtwolame --enable-libvo-aacenc --enable-libvpx --enable-libx264 --enable-libx265 --enable-libxvid --prefix=/usr/local/ffmpeg<br />
$make<br />
$sudo make install<br />
$ln -s /usr/local/ffmpeg/include/lib* /usr/include/<br />

<b>Install</b><br />
$make<br />

<b>Usage</b><br />
//Edit the playlsit file pgmlist.txt. Each line in pgmlist.txt indicate one clip.<br />
//Struct of each line is &quot;&lt;input file location&gt;#&lt;output url&gt;#&lt;frames&gt;&quot;.<br />
//Input files should only be 25fps. &lt;output url&gt; limited to ts muxed multicast streams. &lt;frames&gt; the number of frames to stream.<br />
$vi pgmlist.txt<br />
$./main<br />
