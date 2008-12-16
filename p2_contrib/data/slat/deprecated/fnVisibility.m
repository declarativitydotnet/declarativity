function fv = fnVisibility(x, param)
% fv = fnVisible(x, param)

fx = fnProjection(x, param);
fv = (fx(1,:)<1) & (fx(1,:)>0) & (fx(2,:)<1) & (fx(2,:)>0);
