function result=animate(ev, speed, output)
% animateSlatObservations(problem, speed, output)

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

time = get(ev, 'time');

%index=1;
indexp=1;
%xt=soln.xt;
%Pt=soln.Pt;
next_time=0;
tic;

while length(indexp>1)
  %disp(num2str(t))
  %if t>1
  %  pause(min(problem.time(t)-problem.time(tp-1)-toc,0));
  %end
  
  %t=index(1);
  tp=indexp(1);
  [a,b]=plot(ev, tp);
  subplot(a,b,ceil(b/2));
  title(sprintf('t = %.2fs, step=%d', time(tp), tp));
  drawnow;
  
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
      if t+speed>length(time)
        break;
      else
        next_time=time(t+speed);
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
        next_time=time(max(tp-1,1));
      case 29 % Right
        next_time=time(min(tp+1,length(time)));
      case 30 % Up
        next_time=time(max(tp-10,1));
      case 31 % Down
        next_time=time(min(tp+10,length(time)));
      otherwise
        disp('Invalid key')
        next_time=time(tp);
    end
  end;
  %index=find(time>=next_time);
  indexp=find(time>=next_time);
end

if stills
  result=stills-1;
elseif avi
  result=aviobj;
else
  result=0;
end
