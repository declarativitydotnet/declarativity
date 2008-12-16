function [mu, sigma, likelihood] = approximatePosteriorPF(muX, sigmaX, qmuX, qsigmaX, fn, Q, obs, param)

n=1000;
d=length(muX);
x=sqrtm(qsigmaX)*randn(d,n)+qmuX*ones(1,n);
w=(mvnpdf(x', muX', sigmaX)./mvnpdf(x', qmuX', qsigmaX))';
y=fn(x,param);
w=w.*mvnpdf(y',obs',Q)';
likelihood=sum(w);
w=w/likelihood;
wx=x.*(ones(length(muX),1)*w);
mu=sum(wx,2);
sigma=wx*x'-mu*mu'; 
sigma=(sigma+sigma')/2;
