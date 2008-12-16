function [xt,Pt,xtl,Ptl,cameras]=encode_legacy(model, belief, defaultvar);


if iscell(belief) % distributed solution 
  varl = get_variable('l_t');
  if ~iscell(belief{1})
    mg = marginal(belief{1},locvars(model));
  else
    mg = marginal(decomposable_model(belief{1}),locvars(model));
  end
  xt = mean(mg);
  Pt = sparse(cov(mg));
  
  vars = posevars(model);
  for i=1:size(vars,2)
    if ~iscell(belief{i})
      mg = marginal(belief{i}, vars(:,i));
    else
      mg = marginal(decomposable_model(belief{i}),vars(:,i));
    end
    si = length(xt)+1;
    xt = [xt; mean(mg)];
    ei = length(xt);
    Pt(si:ei,si:ei)=cov(mg);

    if ~iscell(belief{i})
      mg = marginal(belief{i}, varl);
    else
      mg = marginal(decomposable_model(belief{i}),varl);
    end
    xtl{i} = mg.const;
    Ptl{i} = mg.cov;
  end
  
  cameras = 1:size(vars,2);
  
else
  mg = marginal(belief,locvars(model));
  xt = mean(mg);
  Pt = sparse(cov(mg));
  
  vars = posevars(model);
  bvars1 = intersect(args(belief),vars(1,:));
  cameras = find(vars(1,:),bvars1);
  % of course, the indices may not be sorted!!!!
  cameras = sort(cameras);

  if exist('defaultvar','var')
    for i=1:size(vars,2)
      si = length(xt)+1;
      if any(cameras==i)
        mg = marginal(belief, vars(:,i));
        xt = [xt; mean(mg)];
        ei = length(xt);
        Pt(si:ei,si:ei)=cov(mg);
      else
        xt = [xt;zeros(sumdim(vars(:,i)),1)];
        ei = length(xt);
        Pt(si:ei,si:ei) = diag(ones(1,ei-si+1)*defaultvar);
      end
    end        
  else  
    for i=1:length(cameras)
      mg = marginal(belief, vars(:,cameras(i)));
      si = length(xt)+1;
      xt = [xt; mean(mg)];
      ei = length(xt);
      Pt(si:ei,si:ei)=cov(mg);
    end
  end
  xtl = {};
  Ptl = {};
end

