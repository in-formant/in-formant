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

## A mathematical representation of the vocal tract filter

The vocal tract filter can be modeled acoustically as a series of multiple resonances and antiresonances.

We can represent a single resonance as a single-pole filter:

<img src="https://render.githubusercontent.com/render/math?math=H(z) = \frac{G}{1 - a z^{-1}}" />

And a single antiresonance as a single-zero filter:

<img src="https://render.githubusercontent.com/render/math?math=H(z) = G \cdot (1 - b z^{-1})" />

Thus, a series of resonances and antiresonances can be represented as a rational function:

<img src="https://render.githubusercontent.com/render/math?math=H(z) = F_1(z) F_2(z) \cdots F_m(z) \cdot G_1(z) G_2(z) \cdots G_n(z) = G \cdot \frac{\displaystyle (1-b_1 z^{-1})(1-b_2 z^{-1})\cdots(1-b_m z^{-1})}{\displaystyle (1-a_1 z^{-1})(1-a_2 z^{-1})\cdots(1-a_n z^{-1})}" />


