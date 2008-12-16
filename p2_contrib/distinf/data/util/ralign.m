% RALIGN - Rigid alignment of two sets of points in k-dimensional
%          Euclidean space.  Given two sets of points in
%          correspondence, this function computes the scaling,
%          rotation, and translation that define the transform TR
%          that minimizes the sum of squared errors between TR(X)
%          and its corresponding points in Y.  This routine takes
%          O(n k^3)-time.
%
% Inputs:
%   X - a k x n matrix whose columns are points 
%   Y - a k x n matrix whose columns are points that correspond to
%       the points in X
% Outputs: 
%   c, R, t - the scaling, rotation matrix, and translation vector
%             defining the linear map TR as 
% 
%                       TR(x) = c * R * x + t
%
%             such that the average norm of TR(X(:, i) - Y(:, i))
%             is minimized.
%
% See also:
%
%   "Least-Squares Estimation of Transformation Parameters Between
%   Two Point Patterns."  Shinji Umeyama.  IEEE Transactions on
%   Pattern Analysis and Machine Intelligence.  Vol. 13, No. 4,
%   April 1991.

% Copyright (C) 2002 Mark A. Paskin
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
% USA.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [c, R, t] = ralign(X, Y)

[m, n] = size(X);

mx = mean(X, 2);              % Eqn. (34)
my = mean(Y, 2);              % Eqn. (35)

Xc = X - repmat(mx, [1, n]);
Yc = Y - repmat(my, [1, n]);

sx = mean(sum(Xc.^2, 1));     % Eqn. (36)
sy = mean(sum(Yc.^2, 1));     % Eqn. (37)

Sxy = Yc * Xc' ./ n;          % Eqn. (38)

[U, D, V] = svd(Sxy);

r = rank(Sxy);
d = det(Sxy);

S = eye(m);
if (r > m - 1) 
  if (det(Sxy) < 0)
    S(m, m) = -1;
  end
elseif (r == m - 1)
  if (det(U) * det(V) < 0)
    S(m, m) = -1;
  end
else
  warning('Insufficient rank in covariance to determine rigid transform');
  R = [1, 0; 0, 1];
  c = 1;
  t = [0; 0];
  return;
end

R = U * S * V';               % Eqn. (40)
c = trace(D * S) / sx;        % Eqn. (42)
t = my - c * R * mx;          % Eqn. (41)
