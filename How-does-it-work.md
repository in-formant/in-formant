# How this voice analyzer and synthesizer works

---
 
## Some assumptions were made to simplify the modeling

The most fundamental assumption here was the assumption of no source-fiter interactions.
This means that the filter does not feed back into the source, and the source does not feed back into the filter.

As a consequence, this model is unable to represent "uncommon" vocal configurations, such as growling or belting.

Another assumption that was made is the absence of antiresonances in the vocal tract filter.
This is once again inaccurate for certain vocal configurations, such as nasal sounds and certain consonants.

It's also assumed that the glottal source of the sound being analyzed is mostly non-pathological.
This simplifies greatly the algorithms used for pitch tracking and glottal inverse filtering.

## Reformulating the formant analysis problem

The vocal tract filter can be modeled acoustically as a series of multiple resonances and antiresonances.

We can represent a single resonance as a single-pole filter:

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;F_i(z)&space;=&space;\frac&space;{1}&space;{1&space;-&space;a_i&space;z^{-1}}" title="\large F_i(z) = \frac {G_i} {1 - a_i z^{-1}}" />

And a single antiresonance as a single-zero filter:

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;G_i(z)&space;=&space;1&space;-&space;b_i&space;z^{-1}" title="\large G_i(z) = 1 - b_i z^{-1}" />

Thus, a series of resonances and antiresonances can be represented as a rational function:

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;H(z)&space;=&space;\prod_{i&space;=&space;1}^{n}&space;F_i(z)&space;\prod_{j&space;=&space;1}^{m}&space;G_j(z)" title="\large H(z) = \prod_{i = 1}^{n} F_i(z) \prod_{j = 1}^{m} G_j(z)" />

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;H(z)&space;=&space;\frac&space;{(1&space;-&space;b_1&space;z^{-1})&space;(1&space;-&space;b_2&space;z^{-1})&space;\cdots&space;(1&space;-&space;b_m&space;z^{-1})}&space;{(1&space;-&space;a_1&space;z^{-1})&space;(1&space;-&space;a_2&space;z^{-1})&space;\cdots&space;(1&space;-&space;a_n&space;z&space;^{-1})}" title="\large H(z) = \frac {(1 - b_1 z^{-1}) (1 - b_2 z^{-1}) \ldots (1 - b_m z^{-1})} {(1 - a_1 z^{-1}) (1 - a_2 z^{-1}) \ldots (1 - a_n z ^{-1})}" />

Since we assumed there are no antiresonances, we can simplify this into:

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;\bg_white&space;H(z)&space;=&space;\frac&space;{1}&space;{(1&space;-&space;a_1&space;z^{-1})&space;(1&space;-&space;a_2&space;z^{-1})&space;\cdots&space;(1&space;-&space;a_n&space;z&space;^{-1})}" title="\large \bg_white H(z) = \frac {1} {(1 - a_1 z^{-1}) (1 - a_2 z^{-1}) \ldots (1 - a_n z ^{-1})}" />

We can then expand that into the common form of a rational transfer function.

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;\bg_white&space;H(z)&space;=&space;\frac&space;{1}&space;{1&space;-&space;x_1&space;z^{-1}&space;-&space;x_2&space;z^{-2}&space;-&space;\ldots&space;-&space;x_n&space;z^{-n}}" title="\large \bg_white H(z) = \frac {1} {1 - x_1 z^{-1} - x_2 z^{-2} - \ldots - x_n z^{-n}}" />

Based on that expression, we can reformulate the formant analysis problem as such: estimate teh polynomial coefficients of the transfer function denominator.
