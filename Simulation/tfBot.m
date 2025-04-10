% System parameters:

% m(n): Mass of the nth layer
% mr: Mass of the four rods
% r(n): Distance between the rotor and the nth layer
% lr: Length of the rods

m1 = 1.146;  % kg
r1 = 0.03;  % m
m2 = 0.197;  % kg
r2 = 0.085;  % m
m3 = 0.1254;  % kg
r3 = 0.14;  % m
%r3 = 0.25;  % m
mr = 0.222;  % kg
lr = 0.23;  % m

% lg: Distance between the rotor and the center of mass
% Jp: Total moment of inertia of the bot
mp = sum([m1 m2 m3 mr]);  % kg
lg = dot([m1 m2 m3 mr], [r1 r2 r3 lr/2]) / mp;  % m
Jp = dot([m1 m2 m3], ([r1 r2 r3]-lg).^2) + 1/3*mr*(lg^3+(lr-lg)^3)/lr;  % kg m^2

% mw: Mass of the wheel
% rw: Radius of the wheel
mw = 0.02; % kg
rw = 0.04;  % m
Jw = 1/2*mw*rw^2; % kg m^2

% Friction b and moment of inertia of the rotor is estimated to be 0
Kt = 0.1516;
Ke = 0.01146;
Jm = 0;
Gr = 1/18.75;
b = 0;
Ra = 2.1828; % ohms
s = tf("s");

% Derive the transfer function and show the location of the open loop poles
tfFish = minreal((180/pi)*Kt/Ra*(mp*lg*rw)/(((Jp-mp*lg^2)*(Jm/Gr+Jw+(mw-mp)*rw^2)+(mp*lg*rw)^2)*s^2+(Jp-mp*lg^2)*(Kt/Ra*Ke/Gr+b/Gr)*s))
OpenLoopPoles = pole(tfFish)
step(tfFish)
ylim([0 90])
xlim([0 1])
% Calculate the PID gains with Pole Placement method
% N: Time constant 
K = tfdata(tfFish, 'v');
N = 0.25;
Pa = -1/N * 2 * pi;
Pb = -1/N * 2 * pi;

syms x y
Kp = double(solve(Pa * Pb == OpenLoopPoles(1) * OpenLoopPoles(2) + K(3) * x , x))
Kd = double(solve(-(Pa + Pb) == K(3) * y - (OpenLoopPoles(1) + OpenLoopPoles(2)), y))
Ki = 0;
C = Kp+Kd*s+Ki/s;
tfcp = minreal(feedback(tfFish * C,1))
CloseLoopPoles = pole(tfcp)





