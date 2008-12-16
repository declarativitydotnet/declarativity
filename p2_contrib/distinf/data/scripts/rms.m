function err = rms(a, b)
if size(a,1) ~= size(b,1)
  a=repmat(a,size(b,1),1);
end
err = sqrt(mean((a-b).^2, 2));
