function fx = fnLinear(x, param)
[stuff, n] = size(x);
fx = param.A*x+param.b*ones(1,n);

