;No code yet, just thinking on how the hell to do it
;PSG seems quite simple to use, $A0 chooses port, $A1 you write to it, $A2 you read from it
;No page swapping or any silly shit like that
;0-5 are the frequencies (3.579545MHz / 32 / nn) (need to get the notes), and 6 control noise frequency
;7 turns on the notes (c1-3 tone, c1-3 noise,useless I/O stuff)  (so its three channels of noise or 2 channels and noise?)
;8-A 4bit volumes, or envelope mode at 16 (interesting)
;B-C choose envelope frequency, F = 3.579545MHz / 32 / nn (same formula lol)
;D choose the envelope shape (check portar for the table)
;E is actually useful because its the joystick
;F also is at bit 6 because you select the joystick port (just set the thing to 1 lol)

;Now instrument vs sound effect and how to store it
;If we use 32 bytes per instrument, we have 32 instruments in a KB, so lets see if you can pack in 8 or 16
;We could have ADSR (2 bytes?),then 1 byte (pitch program/speed), 4bit echo flange mode offset/4bit echo note offset,then a byte for setting flags
;That's 5 bytes, and it's quite packed of features
;I probably should implement an audio emulator on the editor itself to test those ideas
;But with those 5 bytes for example, 64 instruments would be just 320 bytes

;Now instruments
;The naive way of doing that would be, byte 1 6bit note, 2bit length, byte 2 instrument, byte 3 volume
;Now a stupid idea would be..
;4bit note size,4bit note (it goes just to note 11)
;could do note 13,14,15 is commands such as octave, instrument, volume, but instrument is too limited, unless we use the next byte
;note 11 could be silence
;supposing i make 20 sound effects with this, each with 12 bytes in average
;That's 240 bytes, not bad
;i could stuff sound effects in the level file as well
;All i would have to do is a instruction like
;PlayChannel (HL address, C BPM,D channel)
;Can have also a PlayChannelOverlay that temporarily takes over the music driver to play something
;With some smarter stuff, i could do it alternate between the music and audio channel during the silence
;That would allow me to have a bass line that interrupt the music as well

;Now on the editor itself