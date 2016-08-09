# Denver Streaming Server
Denver Streming Server is a streaming software on Linux platform, released under the GNU General Public License (GPL). The expected inputs of this software are video files encoded by h.264, h.265, vp9, etc. The output is a ts-muxed multicast stream, without changing video/audio formats.<br />

<b>Requirements</b><br />
Install lastest FFmpeg software properly first. In this progress, you may need to install some supporting software.<br />
$git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg<br />
$cd ffmpeg<br />
$./configure --enable-version3 --enable-nonfree --enable-gpl --enable-libfaac --enable-libfdk-aac --enable-libmp3lame --enable-libopenh264 --enable-libopenjpeg --enable-libtwolame --enable-libvo-aacenc --enable-libvpx --enable-libx264 --enable-libx265 --enable-libxvid --prefix=/usr/local/ffmpeg<br />
$make<br />
$sudo make install<br />
$ln -s /usr/local/ffmpeg/include/lib* /usr/include/<br />

<b>Install</b><br />
$make<br />

<b>Usage</b><br />
Edit the playlsit file playlist.txt. Each line in pgmlist.txt indicate one clip. Every clip should only be 25fps.<br /> 
Struct of each line is &quot;&lt;input file location&gt;#&lt;output url&gt;#&lt;frames&gt;&quot;.<br /> 
&lt;output url&gt;: limited to ts-muxed multicast streams. &lt;frames&gt;: the number of frames to stream.<br />
$vi playlist.txt<br />
$./main<br />
