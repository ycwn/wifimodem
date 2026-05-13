#!/usr/bin/env python3


import math


ranked  = []
invalid = []

for ch in range(256):

	symbols = []

	bit = ch & (1 << 7) != 0
	num = 1

	for b in range(6, -1, -1):

		nbit = ch & (1 << b) != 0

		if nbit != bit:
			symbols.append(num)
			num = 1
			bit = nbit

		else:
			num += 1

	symbols.append(num)

	if (ch & 1) == 0:
		symbols[-1] += 1

	else:
		symbols.append(1)

	M = min(symbols)
	S = 0
	N = 0

	for k in symbols:
		S += k
		N += math.floor( float(k) / float(M) + 0.5)

	print(hex(ch), bin(ch), symbols, M, S, N, float(S) / float(N))

	ranked.append( ( float(S) / float(N), ch))

	if S > N:
		invalid.append(ch)



for e, c in sorted(ranked):
	if e > 1.0:
		print(e, c, hex(c))


print(invalid)

print("static const bool autobaud_valid[256] = {")
print("\t" + ",\n\t".join(",".join(" false" if (r+c) in invalid else "  true" for c in range(16)) for r in range(0, 256, 16)))
print("}")


