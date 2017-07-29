load("WAVETABLE")
control_rate(44100)

totdur = 10
dur = 0.3
incr = 0.2
env = maketable("line", 1000, 0,0, 1,1, 20,0)
for (start = 0; start < totdur; start += incr) {
	amp = irand(5000, 15000)
	freq = irand(200, 400)
	pan = irand(0, 1)
	WAVETABLE(start, dur, amp * env, freq, pan)
}

