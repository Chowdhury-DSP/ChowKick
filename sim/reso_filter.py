import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt

FS = 48000.0
worN = np.logspace(1.0, 3.3, base=20, num=200)

def plot_response(w, h, *args, **kwargs):
    plt.semilogx(w, 20 * np.log10(np.abs(h)), *args, **kwargs)

# inverse notch filter
#         s^2 + s/Q + 1
# H(s) = ---------------
#            s^2 + 1
def inverse_notch_s(freq, Q):
    w = freq
    return [1.0 / w**2, 1.0 / w / Q, 1], [1.0 / w**2, 0, 1]

b_s, a_s = inverse_notch_s(100, 0.5)
w, h = signal.freqs(b_s, a_s, worN=worN)
plot_response(w, h)

def inverse_notch_z(freq, Q, fs):
    wc = 2 * np.pi * freq / fs
    wS = np.sin(wc)
    wC = np.cos(wc)
    alpha = wS / (2.0 * Q)

    a0 = 1.0 + alpha
    return [a0, -2.0 * wC, (1.0 - alpha)], [1.0, -2.0 * wC, 1.0]

b_z, a_z = inverse_notch_z(100, 0.5, FS)
w, h = signal.freqz(b_z, a_z, worN=worN, fs=FS)
plot_response(w, h, '--')

plt.title('Inverse Notch Filter')
plt.xlabel('Frequency [Hz]')
plt.ylabel('Amplitude [dB]')
plt.legend(['Analog', 'Digitized'])
plt.grid()

############################################
# resonant filter
#            H(s)
# T(s) = ------------
#         1 + G H(s)
#
#             s^2    +   s/Q    + 1
# T(s) = -----------------------------
#         s^2 (G + 1) + G s/q + G + 1

plt.figure()

def reso_filt_s(freq, Q, G):
    w = freq
    return [1.0 / w**2, 1.0 / w / Q, 1], [(G + 1) / w**2, G / w / Q, G + 1]

b_s, a_s = reso_filt_s(100, 10.0, -0.1)
w, h = signal.freqs(b_s, a_s, worN=worN)
plot_response(w, h)

def reso_filt_z(freq, Q, G, fs):
    wc = 2 * np.pi * freq / fs
    wS = np.sin(wc)
    wC = np.cos(wc)
    alpha = wS / (2.0 * Q)

    a0 = (G + 1.0) + alpha * G
    b = [(1 + alpha) / a0, -2 * wC / a0, (1 - alpha) / a0]
    a = [1.0, -2 * wC * (G + 1) / a0, ((G + 1) - G * alpha) / a0]
    return b,a

b_z, a_z = reso_filt_z(100, 10.0, -0.1, FS)
w, h = signal.freqz(b_z, a_z, worN=worN, fs=FS)
plot_response(w, h, '--')

plt.title('Resonant Filter')
plt.xlabel('Frequency [Hz]')
plt.ylabel('Amplitude [dB]')
plt.legend(['Analog', 'Digitized'])
plt.grid()

plt.show()
