function plot(prob, varargin)

[printable, rest] = process_options(varargin, 'printable', 0);

plotcams(prob, varargin{:})

if printable
  fontsize=16;
  fontname='times';
else
  fontsize=10;
  fontname='helvetica';
end

hold on;
  
steps = [1 prob.nsteps]; %[1 20:20:prob.nsteps];
h=plot(prob.pos(1,:),prob.pos(2,:),'r.');
%set(h, 'MarkerSize', 10);
for t=steps,
  h=text(prob.pos(1,t),prob.pos(2,t)+0.05,['\it{t} = ' num2str(t)]);
  %set(h,'backgroundcolor',[1 1 1]);
  set(h, 'fontsize', fontsize);
  set(h, 'fontname', fontname);
  set(h, 'verticalalignment', 'baseline');
end

%ppos=invertProjection(prob.obs(:,:,3),prob.calib(3),0,0,[0 0 1 -1.8]);
%plot(ppos(1,:),ppos(2,:),'m-');
hold off;
