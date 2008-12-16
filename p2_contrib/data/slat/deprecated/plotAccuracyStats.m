function klall=plotAccuracyStats(prob, model, vars, kf, varargin)
% plotAccuracyStats(prob, model, vars, kf, varargin)
% vars - list of variables to show KL, MSE, etc. over
colors='brmgcy:.';

args=[];
for i=1:length(varargin)
  args=[args varargin{i}];
end

if length(find(vars==0))==0
figure(8);
clf;
hold on;
for i=1:length(args)
  data=args(i);
  nt=size(data.xt,2);
  for t=1:nt,
    kl(t)=0;
    for k=1:prob.nCams,
      ci=model.ci(k,vars);
      kl(t)=kl(t)+gaussianKL(kf.xt(ci,t), kf.Pt{t}(ci,ci), ...
          data.xt(ci,t),data.Pt{t}(ci,ci));
    end
  end
  klall(i,:)=kl;
  plot(kl,colors(i));
end
hold off
ylabel('KL divergence from the exact posterior');
xlabel('time step');
legend(args.name);
else
  vars=vars(find(vars~=0));
end

colors=['k' colors];
args=[kf args];
figure(9);
clf
hold on;
nvars=length(vars);
for i=1:length(args)
  data=args(i);
  nt=size(data.xt,2);
  seen=zeros(model.nCams,nt);
  for ti=1:nt,
    ci=model.ci(:,1);
    d=diag(data.Pt{ti});
    seen(:,ti)=d(ci)<90;
  end
  for k=1:model.nCams,
    ci=model.ci(k,vars);
    diff((k-1)*nvars+(1:nvars),:) = ...
      data.xtc(ci,:)-(model.xt(ci,:)*seen(k,:));
  end
  rms=sqrt(sum(diff.*diff,1)./sum(seen,1)/nvars);   
  plot(rms,colors(i));
end
hold off;
ylabel('rms');
xlabel('time step');
if isfield(args(1), 'name'),
  legend(args.name);
end

colors=['k' colors];
args=[kf args];
figure(10);
clf
hold on;
nvars=length(vars);
for i=1:length(args)
  data=args(i);
  nt=size(data.xt,2);
  seen=zeros(model.nCams,nt);
  for ti=1:nt,
    ci=model.ci(:,1);
    d=diag(data.Pt{ti});
    seen(:,ti)=d(ci)<90;
  end
  for k=1:model.nCams,
    ci=model.ci(k,vars);
    diff((k-1)*nvars+(1:nvars),:) = ...
      data.xtr(ci,:)-(model.xt(ci,:)*seen(k,:));
  end
  rms=sqrt(sum(diff.*diff,1)./sum(seen,1)/nvars);   
  plot(rms,colors(i));
end
hold off;
ylabel('rms');
xlabel('time step');
if isfield(args(1), 'name'),
  legend(args.name);
end

