function result=animateSlatSolution(problem, model, soln, options, speed, output)
% result=animateSlatSolution(problem, model, soln, options, speed, output)
% output can be either filename base (for stills) or an AVI object
%
% Options are any options that plotSlatSolution takes.
%
% Defaults: defa

if ~exist('options', 'var')
  options='defa';
end

if ~exist('speed', 'var')
  speed=1;
end

stills=0;
avi=0;

if exist('output', 'var')
  if ischar(output)
    stills=1;
  else
    avi=1;
    aviobj=output;
    fps=get(aviobj, 'fps');
  end
else
  set(gcf, 'SelectionType', 'open');
end

%Compute the optimal rigid transform
if any(options=='r') & ~isfield(soln, 'transform')
  soln.transform=slatComputeOptimalTransform(problem, model, soln);
end

index=1;
indexp=1;
next_time=0;
running=1;
tic;

if ~isfield(soln, 'time')
  soln.time=problem.time;
end

last=0;

while length(index>1)
  t=index(1);
%   tp=indexp(1);
%   if tp>problem.nSteps
%     break;
%   end

  plotSlatSolution(problem, model, soln, t, options); 
  
  % Save the output
  if stills
    print('-dpng', sprintf('%s%03d.png', output, stills));
    stills=stills+1;
  elseif avi
    aviobj=addframe(aviobj, gca);
    avi=avi+1;
  end
  
  % do a simple speed control
  if speed>0
    if stills
      if t+speed>length(soln.time)
        break;
      else
        next_time=soln.time(t+speed);
      end
    elseif avi
      next_time=avi/fps*speed;
    else
      %drawnow;
      pause(0.01);
      if strcmp(get(gcf,'SelectionType'),'normal')
        button=waitforbuttonpress;
        while button~=0 || ~strcmp(get(gcf,'SelectionType'), 'normal')
          button=waitforbuttonpress;
        end
        set(gcf, 'SelectionType', 'open');
        %while get(gcf,'CurrentCharacter')~=' '
        %%  waitforbuttonpress;
        %end
        tic;
      else
        next_time=next_time+toc*speed;
        tic;
      end
    end
  else
    buttontype=waitforbuttonpress;
    switch double(get(gcf,'CurrentCharacter'))
      case 27 % Escape
        break;     
      case 28 % Left
        next_time=soln.time(max(t-1,1));
      case 29 % Right
        next_time=soln.time(min(t+1,length(soln.time)));
      case 30 % Up
        next_time=soln.time(max(t-10,1));
      case 31 % Down
        next_time=soln.time(min(t+10,length(soln.time)));
      otherwise
        disp('Invalid key')
        next_time=soln.time(t);
    end
  end;
  index=find(soln.time>=next_time);
  %indexp=find(problem.time>=next_time-soln.burn_in-1e-8);
  if length(index)==0 && last==0,
    last=1;
    index=problem.nSteps;
    %indexp=problem.nSteps;
  end
end

if stills
  result=stills-1;
elseif avi
  result=aviobj;
else
  result=0;
end
