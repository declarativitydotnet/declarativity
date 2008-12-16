%
% [idx,netsim,dpsim,expref]=apcluster(s,p)  Demo: Type "apcluster <enter>"
%
% APCLUSTER uses affinity propagation (BJ Frey and D Dueck, Science, 2007)
% to identify exemplars and clusters, using a set of real-valued pairwise
% data point similarities as input. Each cluster is represented by a data
% point called an exemplar, and the method iteratively searches for
% clusters so as to maximize an objective function, called net similarity.
% The command apcluster(s,p,'plot') plots the objective function.
% 
% For N data points, there are potentially N^2-N pairwise similarities, 
% which can be input as an N-by-N matrix 's', where s(i,k) is the 
% similarity of point i to point k (note: s(i,k) need not equal s(k,i)).
% In fact, only a smaller number of relevant similarities are needed; if 
% only M similarity values are known, they can be input as an M-by-3
% matrix with each row being an (i,j,s(i,j)) triple (eg, type "apcluster"
% without arguments). Other similarities are assumed to equal -Inf.
% 
% APCLUSTER automatically determines the number of clusters based on the
% input 'p', which may be a real number or a vector of N numbers. p(i)
% indicates the preference that data point i be chosen as an exemplar and
% if p is scalar, APCLUSTER assumes all preferences are equal to p. Try
% setting p to the median similarity or the minimum similarity; the number
% of clusters can be increased or decreased by increasing or decreasing p.
% The function PREFERENCERANGE computes the minimum and maximum preference
% values appropraite for a provided set of similarities.
% 
% The solution is returned in idx. idx(j) is the index of the exemplar for
% data point j and idx(j)==j if data point j is itself an exemplar. dpsim 
% is the sum of the similarities of the data points to their exemplars,
% expref is the sum of the exemplar preferences, and netsim is the net
% similarity (objective function), which equals dpsim + expref.
% 
% 	apcluster(s,p,'NAME',VALUE,...) is used to specify other parameters:
% 
%   'maxits'     Maximum number of iterations (default: 1000)
%   'convits'    If the exemplars stay fixed for convits iterations,
%                APCLUSTER terminates early (default: 100)
%   'dampfact'   Damping factor between 0.5 and 1. Heavy damping (higher 
%                values) suppresses oscillations (default: 0.9)
%   'plot'       (No VALUE needed) Plots netsim after each iteration
%   'details'    (No VALUE needed) Causes all intermediate results to be
%                stored in the output variables (may use excessive memory)
%   'nonoise'    (No VALUE needed) APCLUSTER adds a tiny amount of noise 
%                to the similarities to avoid degeneracies (see Science
%                paper for details); this turns off the addition of noise
% 
% Copyright (c) B.J. Frey & D. Dueck (2006,2007). This software may be 
% freely used and distributed for non-commercial purposes only.
%
%  *** Type "apcluster" without arguments for a demonstration ***
function [idx,netsim,dpsim,expref,A,R]=apcluster(s,p,varargin);

if nargin==0, % display demo
	fprintf('\nAffinity propagation (APCLUSTER) sample/demo code: Copy\n');
    fprintf('and paste this into your MATLAB session to see the demo.\n\n');
	fprintf('N=100; x=rand(N,2); %% Create N, 2-D data points\n');
	fprintf('M=N*N-N; s=zeros(M,3); %% Make ALL N^2-N similarities\n');
	fprintf('j=1;\n');
	fprintf('for i=1:N\n');
	fprintf('  for k=[1:i-1,i+1:N]\n');
	fprintf('    s(j,1)=i; s(j,2)=k; s(j,3)=-sum((x(i,:)-x(k,:)).^2);\n');
	fprintf('    j=j+1;\n');
	fprintf('  end;\n');
	fprintf('end;\n');
	fprintf('p=median(s(:,3)); %% Set preference to median similarity\n');
	fprintf('[idx,netsim,dpsim,expref]=apcluster(s,p,''plot'');\n');
	fprintf('\nfigure; %% Make a figures showing the data and the clusters\n');
	fprintf('for i=unique(idx)''\n');
	fprintf('  ii=find(idx==i); h=plot(x(ii,1),x(ii,2),''o''); hold on;\n');
	fprintf('  col=rand(1,3); set(h,''Color'',col,''MarkerFaceColor'',col);\n');
	fprintf('  xi1=x(i,1)*ones(size(ii)); xi2=x(i,2)*ones(size(ii)); \n');
	fprintf('  line([x(ii,1),xi1]'',[x(ii,2),xi2]'',''Color'',col);\n');
	fprintf('end;\n');
	fprintf('axis equal tight;\n');
    fprintf('title([''Net similarity: '' num2str(netsim)]);\n\n');
	return;
end;
start = clock;
% Handle arguments to function
if nargin<2 error('Too few input arguments');
else
    maxits=1000; convits=100; lam=0.9; plt=0; details=0; nonoise=0;
    i=1;
    while i<=length(varargin)
        if strcmp(varargin{i},'plot')
            plt=1; i=i+1;
        elseif strcmp(varargin{i},'details')
            details=1; i=i+1;
		elseif strcmp(varargin{i},'sparse')
% 			[idx,netsim,dpsim,expref]=apcluster_sparse(s,p,varargin{:});
			fprintf('''sparse'' argument no longer supported; see website for additional software\n\n');
			return;
        elseif strcmp(varargin{i},'nonoise')
            nonoise=1; i=i+1;
        elseif strcmp(varargin{i},'maxits')
            maxits=varargin{i+1};
            i=i+2;
            if maxits<=0 error('maxits must be a positive integer'); end;
        elseif strcmp(varargin{i},'convits')
            convits=varargin{i+1};
            i=i+2;
            if convits<=0 error('convits must be a positive integer'); end;
        elseif strcmp(varargin{i},'dampfact')
            lam=varargin{i+1};
            i=i+2;
            if (lam<0.5)||(lam>=1)
                error('dampfact must be >= 0.5 and < 1');
            end;
        else i=i+1;
        end;
    end;
end;
if lam>0.9
    fprintf('\n*** Warning: Large damping factor in use. Turn on plotting\n');
    fprintf('    to monitor the net similarity. The algorithm will\n');
    fprintf('    change decisions slowly, so consider using larger values\n');
    fprintf('    for convits and maxits.\n\n');
end;

% Check that standard arguments are consistent in size
if length(size(s))~=2 error('s should be a 2D matrix');
elseif length(size(p))>2 error('p should be a vector or a scalar');
elseif size(s,2)==3
    tmp=max(max(s(:,1)),max(s(:,2)));
    if length(p)==1 N=tmp; else N=length(p); end;
    if tmp>N
        error('data point index exceeds number of data points');
    elseif min(min(s(:,1)),min(s(:,2)))<=0
        error('data point indices must be >= 1');
    end;
elseif size(s,1)==size(s,2)
    N=size(s,1);
    if (length(p)~=N)&&(length(p)~=1)
        error('p should be scalar or a vector of size N');
    end;
else error('s must have 3 columns or be square'); end;

% Construct similarity matrix
if N>3000
    fprintf('\n*** Warning: Large memory request. Consider using the\n');
    fprintf('    compiled Windows or Linux version of APCLUSTER.\n\n');
end;
if size(s,2)==3
    S=-Inf*ones(N,N); 
    for j=1:size(s,1) S(s(j,1),s(j,2))=s(j,3); end;
else S=s;
end;

if S==S', symmetric=true; else symmetric=false; end;

% In case user did not remove degeneracies from the input similarities,
% avoid degenerate solutions by adding a small amount of noise to the
% input similarities
if ~nonoise
    rns=randn('state'); randn('state',0);
    S=S+(eps*S+realmin(class(s))*100).*rand(N,N);
    randn('state',rns);
end;

% Place preferences on the diagonal of S
if length(p)==1 for i=1:N S(i,i)=p; end;
else for i=1:N S(i,i)=p(i); end;
end;

% Numerical stability -- replace -INF with -realmin
n=find(S<-realmax); if ~isempty(n), warning('-INF similarities detected; changing to -REALMAX to ensure numerical stability'); S(n)=-realmax; end; clear('n');
if ~isempty(find(S>realmax,1)), error('+INF similarities detected; change to a large positive value (but smaller than +REALMAX)'); end;


% Allocate space for messages, etc
dS=diag(S); A=zeros(N,N,class(s)); R=zeros(N,N,class(s)); t=1;
if plt, netsim=zeros(1,maxits+1); end;
if details
    idx=zeros(N,maxits+1);
    netsim=zeros(1,maxits+1); 
    dpsim=zeros(1,maxits+1); 
    expref=zeros(1,maxits+1); 
end;

% Execute parallel affinity propagation updates
e=zeros(N,convits); dn=0; i=0;
if symmetric, ST=S; else ST=S'; end; % saves memory if its symmetric
while ~dn
    i=i+1; 

    % Compute responsibilities
	A=A'; R=R';
	for ii=1:N,
		old = R(:,ii);
        AS = A(:,ii) + ST(:,ii);
        [Y,I]=max(AS); 
		AS(I)=-Inf;
        [Y2,I2]=max(AS);
        R(:,ii)=ST(:,ii)-Y;
        R(I,ii)=ST(I,ii)-Y2;
		R(:,ii)=(1-lam)*R(:,ii)+lam*old; % Damping
	end;
	A=A'; R=R';

    % Compute availabilities
	for jj=1:N,
		old = A(:,jj);
        Rp = max(R(:,jj),0);
        Rp(jj)=R(jj,jj);
        A(:,jj) = sum(Rp)-Rp;
        dA = A(jj,jj);
        A(:,jj) = min(A(:,jj),0) ;
        A(jj,jj) = dA;
		A(:,jj) = (1-lam)*A(:,jj) + lam*old; % Damping
	end;
	
    % Check for convergence
    E=((diag(A)+diag(R))>0); e(:,mod(i-1,convits)+1)=E; K=sum(E);
    if i>=convits || i>=maxits,
        se=sum(e,2);
        unconverged=(sum((se==convits)+(se==0))~=N);
        if (~unconverged&&(K>0))||(i==maxits) dn=1; end;
    end;

    % Handle plotting and storage of details, if requested
    if plt||details
        if K==0
            tmpnetsim=nan; tmpdpsim=nan; tmpexpref=nan; tmpidx=nan;
        else
            I=find(E); notI=find(~E); [tmp c]=max(S(:,I),[],2); c(I)=1:K; tmpidx=I(c);
            tmpdpsim=sum(S(sub2ind([N N],notI,tmpidx(notI))));
            tmpexpref=sum(dS(I));
            tmpnetsim=tmpdpsim+tmpexpref;
        end;
    end;
    if details
        netsim(i)=tmpnetsim; dpsim(i)=tmpdpsim; expref(i)=tmpexpref;
        idx(:,i)=tmpidx;
    end;
    if plt,
        netsim(i)=tmpnetsim;
		figure(234);
        tmp=1:i; tmpi=find(~isnan(netsim(1:i)));
        plot(tmp(tmpi),netsim(tmpi),'r-');
        xlabel('# Iterations');
        ylabel('Fitness (net similarity) of quantized intermediate solution');
%         drawnow; 
    end;
end; % iterations
I=find((diag(A)+diag(R))>0); K=length(I); % Identify exemplars
if K>0
    [tmp c]=max(S(:,I),[],2); c(I)=1:K; % Identify clusters
    % Refine the final set of exemplars and clusters and return results
    for k=1:K ii=find(c==k); [y j]=max(sum(S(ii,ii),1)); I(k)=ii(j(1)); end; notI=reshape(setdiff(1:N,I),[],1);
    [tmp c]=max(S(:,I),[],2); c(I)=1:K; tmpidx=I(c);
	tmpdpsim=sum(S(sub2ind([N N],notI,tmpidx(notI))));
	tmpexpref=sum(dS(I));
	tmpnetsim=tmpdpsim+tmpexpref;
else
    tmpidx=nan*ones(N,1); tmpnetsim=nan; tmpexpref=nan;
end;
if details
    netsim(i+1)=tmpnetsim; netsim=netsim(1:i+1);
    dpsim(i+1)=tmpdpsim; dpsim=dpsim(1:i+1);
    expref(i+1)=tmpexpref; expref=expref(1:i+1);
    idx(:,i+1)=tmpidx; idx=idx(:,1:i+1);
else
    netsim=tmpnetsim; 
    %dpsim=tmpdpsim; 
    dpsim=1;
    expref=tmpexpref; 
    idx=tmpidx;
end;
if plt||details
    fprintf('\nNumber of exemplars identified: %d  (for %d data points)\n',K,N);
    fprintf('Net similarity: %f\n',tmpnetsim);
    fprintf('  Similarities of data points to exemplars: %f\n',dpsim(end));
    fprintf('  Preferences of selected exemplars: %f\n',tmpexpref);
    fprintf('Number of iterations: %d\n\n',i);
	fprintf('Elapsed time: %f sec\n',etime(clock,start));
end;
if unconverged
	fprintf('\n*** Warning: Algorithm did not converge. Activate plotting\n');
	fprintf('    so that you can monitor the net similarity. Consider\n');
	fprintf('    increasing maxits and convits, and, if oscillations occur\n');
	fprintf('    also increasing dampfact.\n\n');
end;

