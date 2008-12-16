function h=draw_arc(from, to, center, radius)

N = 30;

alpha = linspace(from,to,N);
h = line(radius*cos(alpha)+center(1),radius*sin(alpha)+center(2));