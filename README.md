# Magnetic levitron

Main goal
-------
Make a nice gift for fried. Based on avr Atmega8 and avr-gcc toolchain.

Construction
-------
* PD2 (INT0) - Calibrate button
* PB0        - led (anod)
* PB1 (OC1A) - PWM output for feild transistor
* PC3 (ADC3) - variable resistor
* PC4 (ADC4) - signal from magnetic hall sensors (diff with OPAMP LM324)
