function [xt,Pt,xtl,Ptl] = legacy(soln, step, type, error_on_fail);
% [xt,Pt] = legacy(soln)
% computes the legacy representation of the solution

if nargin<3
  type='posterior';
end

if nargin<4
  error_on_fail=1;
end

s = get(soln);

if ~isfield(s,'xt') || isempty(s.xt)
  model = s.experiment.model;
  belief = get_belief(soln, step, type, error_on_fail);
  if isempty(belief)
    xt=[];Pt=[];xtl=[];Ptl=[];
  else
    [xt,Pt,xtl,Ptl]=encode_legacy(model, belief);
  end
else
  t = step;
  xt=s.xt(:,t);
  Pt=s.Pt{t};
  xtl={};
  Ptl={};
  for k=1:length(s.xtl)
    xtl{k} = s.xtl{k}(:,t);
    Ptl{k} = s.Ptl{k}(:,:,t);
  end
end


  

