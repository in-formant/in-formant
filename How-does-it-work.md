# How this voice analyzer and synthesizer works
 
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

<img src="https://latex.codecogs.com/svg.latex?\bg_white&space;\fn_cm&space;\large&space;\bg_white&space;H(z)&space;=&space;\frac&space;{1}&space;{1&space;-&space;\alpha_1&space;z^{-1}&space;-&space;\alpha_2&space;z^{-2}&space;-&space;\ldots&space;-&space;\alpha_n&space;z^{-n}}" title="\large \bg_white H(z) = \frac {1} {1 - \alpha_1 z^{-1} - \alpha_2 z^{-2} - \ldots - \alpha_n z^{-n}}" />

Based on that expression, we can reformulate the formant analysis problem as such: estimate the polynomial coefficients of the transfer function denominator.

## Autoregressive model and linear prediction

The previously defined transfer function can be expressed in a time-domain recursive relation.

<img src="https://latex.codecogs.com/svg.latex?\fn_cm&space;\large&space;\bg_white&space;y_k&space;=&space;x_k&space;-&space;\sum_{j&space;=&space;1}^{n}&space;{\alpha_j&space;y_{k&space;-&space;j}}" title="\large \bg_white y_k = x_k - \sum_{j = 1}^{n} {\alpha_j y_{k - j}}" />

We get an expression that corresponds to an autoregressive model of order n, where n is the number of poles in the filter.

This is also called a linear predictive model of order n.

There are several known methods to finding the coefficients of an AR(n) model: autocorrelation, covariance, least squares...

In the case of formant analysis the Burg method (maximum entropy spectral analysis) is preferred for its better stability.

## Relationship between the transfer function polynomial and the resonances

With the linear prediction coefficients estimated, we also have our transfer function polynomial coefficients, which we now need to solve.

The accuracy of the polynomial root finder is absolutely key to the accuracy of the formant estimation.

In this particular implementation, the Laguerre root finding method is used for its simplicity and great accuracy. Since we're only dealing with relatively small polynomials, the tradeoff in time spent is acceptable.

To help us visualize the relationship between the polynomial roots and the estimated resonances, we can plot them on a complex unit circle:

<img src="https://www.ee.columbia.edu/~dpwe/e4810/matlab/pezdemo/help/images/pzplot.png" />

Since we are working in the Z-domain, the argument of the polynomial roots correspond to the pole frequencies.

<img src="https://latex.codecogs.com/svg.latex?\fn_cm&space;\large&space;\bg_white&space;\omega&space;=&space;\frac&space;{2&space;\pi&space;f}{f_s}&space;\Leftrightarrow&space;f&space;=&space;\frac&space;{\omega&space;f_s}{2&space;\pi}" title="\bg_white \omega = \frac {2 \pi f}{f_s} \Leftrightarrow f = \frac {\omega f_s}{2 \pi}" />

And the magnitude of the polynomial roots correspond to the pole bandwidth.

<img src="https://latex.codecogs.com/svg.latex?\fn_cm&space;\large&space;\bg_white&space;r&space;=&space;\text{e}&space;^&space;{-&space;\frac{\pi&space;\Delta&space;f}{f_s}}&space;\Leftrightarrow&space;\Delta&space;f&space;=&space;-\frac{f_s}{\pi}&space;\ln&space;r" title="\large \bg_white r = \text{e} ^ {- \frac{\pi \Delta f}{f_s}} \Leftrightarrow \Delta f = -\frac{f_s}{\pi} \ln r" />

