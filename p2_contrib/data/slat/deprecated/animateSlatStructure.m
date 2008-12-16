function aviobj2=animateSlatStructure(problem, Pt, aviobj)
%function aviobj2=animateSlatStructure(problem, Pt, aviobj)

%mu=zeros(0,problem.nCams);
%sigma=zeros(0,0,problem.nCams);

if exist('aviobj', 'var')
  stepsize=5;
  aviobj2=aviobj;
else
  stepsize=10;
  aviobj2=[];
end

%index = [1:4 5:103 136:-1:104];

figure(1);
maxInv=[];

for t=[1 stepsize:stepsize:problem.nSteps],
  %t
  %if t>1
  %  pause(min(problem.time(t)-problem.time(t-1)-toc,0));
  %end
  
  tic;
  im=max(log10(abs(inv(Pt{t}))),0);
  maxInv=[maxInv max(max(im))];
  imshow(im, [0 5])%, 'XData', [-1.3 problem.nCams-0.1], ...
      %'YData', [-1.3 problem.nCams-0.1]);
  axis on;
  grid on;
  set(gca, 'XColor', 'b', 'YColor', 'b');
  title(sprintf('t = %.2fs', problem.time(t)));
  set(gcf, 'Position', [189 103 952 782]);
  colorbar;
  drawnow;
  if exist('aviobj', 'var')
    aviobj2=addframe(aviobj2, gca);
  end
  
  %ginput(1);
end

figure(2);
semilogy([1 stepsize:stepsize:problem.nSteps],10.^maxInv);
xlabel('t');
ylabel('max P_t^{-1})');
