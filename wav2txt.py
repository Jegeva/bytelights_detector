#! /usr/bin/python3

import wave

def chunks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def ifbl(n):
    return int.from_bytes(n,byteorder='little')

wr = wave.open("DefconIsCanceled.wav")
t = 0
p = 1.0/wr.getframerate()
n = wr.getnframes()

#print(wr.getparams())
#print("t:%f"%(wr.getnframes()/wr.getframerate()))

for i in [ifbl(i) for i in chunks(wr.readframes(n),wr.getsampwidth())]:
    print("%f\t%f" % (t,(i/65535)*0.05))
    t += p
