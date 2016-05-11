from features import mfcc
from features import logfbank
import scipy.io.wavfile as wav

(rate,sig) = wav.read("psychology_short.wav")

#mfcc_feat = mfcc(sig,rate);

winstep = 0.01 
winlen = 0.025
#mfcc_feat = mfcc(sig,rate, winlen, winstep)
fbank_feat = logfbank(sig,rate, winlen, winstep)
#print type(fbank_feat)
#print type(mfcc_feat)

#print fbank_feat.shape[0], fbank_feat.shape[1]
#print mfcc_feat.shape

#print (len(fbank_feat));
#print fbank_feat[100:102,:]

n = fbank_feat.shape[0]
m = fbank_feat.shape[1]

#fo = open("foo.txt", "w")
#print "Name of the file: ", fo.name
#print "Closed or not : ", fo.closed
#print "Opening mode : ", fo.mode
#print "Softspace flag : ", fo.softspace

fo = open("audio_feat_short.txt", "w")

fo.write(' '.join(map(str, (n, m, winstep, winlen))) + "\n");
#fo.write(str(n) + " " + str(m) + " ", "\n");
for i in range(0, n):
    res = str(winstep * i) + ": ";
    for j in range(0, m):
        res = res + " %.3lf " % fbank_feat[i][j]
    #+ ' '.join(map(str, fbank_feat[i])) + "\n")
    res += "\n";
    fo.write(res)



