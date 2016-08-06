# Denver Streaming Server
--------------------------------------------------
Requirements
//Install lastest FFmpeg software properly first.
//In this progress, you may need to install some supporting software.
$./configure --enable-version3 --enable-nonfree --enable-gpl --enable-libfaac --enable-libfdk-aac --enable-libmp3lame --enable-libopenh264 --enable-libopenjpeg --enable-libtwolame --enable-libvo-aacenc --enable-libvpx --enable-libx264 --enable-libx265 --enable-libxvid --prefix=/usr/local/ffmpeg
$make
$sudo make install
$ln -s /usr/local/ffmpeg/include/lib* /usr/include/	

--------------------------------------------------
Install
$make

--------------------------------------------------
Usage
/*Edit the playlsit file pgmlist.txt.
  Each line in pgmlist.txt indicate one clip.
  Struct of each line is "<input file location>#<output url>#<frames>".
  <output url> limited to ts muxed multicast streams.
  <frames> limited to 25fps. So input file should only be 25fps.
*/
$vi pgmlist.txt
$./main
