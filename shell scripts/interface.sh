#! /bin/sh -
# interface.sh - Wifi Radio User Interface Script
# 01/29/09	Jeff Keyzer	http://mightyohm.com
#
# This shell script sets up a playlist in mpd and changes playlist entries
# based on the position of a tuner knob connected to an AVR on the router's
# serial port.
#
# The script expects the AVR to send data at 9600 baud (8N1) to the router,
# in the format "tuner: value", one line at a time.
#
# This script also launches display.sh, the LCD display script.
#
# For more information, visit
# http://mightyohm.com/blog/tag/wifiradio/
#
# This work is protected by the
# Creative Commons Attribution-Share Alike 3.0 United States License.
# http://creativecommons.org/licenses/by-sa/3.0/us/ 

# Some configuration settings
VOLUME=100

trap 'kill $! ; exit 1' SIGINT	# exit on ctrl-c, useful for debugging
				# kills the display.sh process before exiting

stty -echo < /dev/tts/0		# set serial port to 9600 baud
				# so we can talk to the AVR
				# turn off local echo to make TX/RX directions
				# completely separate from each other

# mpd setup
mpc volume $VOLUME	# adjust this to suit your speakers/amplifier
mpc clear	# clear current playlist

# build a playlist, substitute your favorite radio stations here
# the first line becomes station #1, and so on.
mpc add http://rbb.ic.llnwd.net/stream/rbb_fritz_mp3_m_a
mpc add http://1live.akacast.akamaistream.net/7/706/119434/v1/gnl.akacast.akamaistream.net/1live
mpc add http://149.5.240.22/WR-DE-WR57
mpc add http://mp3stream1.apasf.apa.at:8000
mpc add http://chillizetmp3-02.eurozet.pl:8400/
mpc add http://scfire-ntc-aa03.stream.aol.com:80/stream/1025	# di.fm Electro House
mpc add http://c22033-l.i.core.cdn.streamfarm.net/22007mdrfigaro/live/3087mdr_figaro/live_de_128.mp3
mpc add http://c22033-l.i.core.cdn.streamfarm.net/22001mdr1sachsen/live/3087mdr_sachsen/live_de_128.mp3
mpc add http://dradio.ic.llnwd.net/stream/dradio_dkultur_m_a
mpc add http://dradio.ic.llnwd.net/stream/dradio_dlf_m_a
mpc add http://dradio.ic.llnwd.net/stream/dradio_dwissen_m_a
mpc add http://streaming1.fueralle.org:8000/coloradio_160.mp3

mpc playlist	# show the resulting playlist
mpc play

oldstation=1	# var to keep track of what station we're playing

# Tell the AVR we're ready to start doing stuff
echo "AVR Start" > /dev/tts/0

# launch LCD display routines in the background
/root/display2.sh &

while true	# loop forever
do
   inputline="" # clear input
  
   # Loop until we get a valid tuner position from the AVR 
   until inputline=$(echo $inputline | grep -e "^tuner: ")
   do
      inputline=$(head -n 1 < /dev/tts/0)
   done
   value=$(echo $inputline | sed 's/tuner: //')	# strip out the tuner: part
   echo "$value"
  
   # compute playlist positon based on tuner value
   # the tuner range is 0 to MAXADC
   # the ADC range is separated into equally sized bins, each bin
   # is one station
   # station ends up being an integer from 1 to STATIONS
   # that determines what station to play 
   # station=$(expr $value / $ADCBIN + 1)
   station=$value
   # if station has changed, we need to change position in the playlist 
   if [ "$station" -ne "$oldstation" ]
   then
   	echo "Tuner Position: " $value
   	echo "New station..."
   	mpc play $station
    fi
    
   oldstation=$station	# remember what station we're on so we know
   			# if we need to change stations next time around
done

