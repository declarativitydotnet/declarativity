function [mu, sigma, muY, sigmaY, covXY] = gConditional2Joint(muX, sigmaX, a0, A, Q)
% [mu, sigma, muY, sigmaY, covXY] = gConditional2Joint(muX, sigmaX, a0, A, Q)
% Computes the joint over normal variables/vectors (X,Y) 
% where X   ~ N(muX, sigmaX)
%   and Y|X ~ N(a0 + A*X, Q)

muY    = a0  + A*muX;
sigmaY = A*sigmaX*A' + Q;
covXY  = sigmaX*A';

mu     = [muX;muY];
sigma  = [sigmaX covXY;covXY' sigmaY];
