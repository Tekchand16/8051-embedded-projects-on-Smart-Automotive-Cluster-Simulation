 8051-Based Smart Automotive Instrument Cluster Simulation

 A Proteus-simulated 8051 microcontroller project that models a smart automotive instrument cluster. Features include real-time speed calculation using Timer1 in counter mode, temperature sensing via LM35 and ADC0804, and simulated fuel monitoring with Timer0. LCD display shows speed, temperature, and fuel level with interrupt-driven system control.


✅  Simulation Steps-

🔘 Step 1: System Toggle via Push Button
Press button connected to P3.2

Triggers INT0: toggles system ON

Press again: system OFF → LCD stops updating

🌡️ Step 2: Temperature Simulation
Adjust LM35 input via potentiometer or voltage source

0.4V → 40°C

If voltage > 0.4V → LED on (overheat warning)

Reduce voltage < 0.4V → LED turns off

🏎️ Step 3: Speed Simulation
Enable pulse generator to send pulses to P3.5

20 pulses = 1 revolution

Code reads pulse count via Timer1 as external counter

Speed = calculated and shown on LCD

⛽ Step 4: Fuel Simulation
Timer0 simulates fuel reduction every ~1s

When fuel <= 20, LCD shows LowFuel

When fuel < 10:

LCD shows LowFuel

Speed = 0 (TR1 = 0)

Stops pulse counting

