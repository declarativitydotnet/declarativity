function plotimage(img, result)
% plotimage(img, result)
% plotimage(prob, algorithm)

if ischar(img)
  r = loadsoln(result, img);
  load(img); % overwrites img
  plotimage(img,r);
  return;
end
  
bel = result.beliefs(end,:);
  
if size(bel,1) == 1
  bel = reshape(bel, img.rows, img.cols);
else
  assert(size(bel,1)==img.rows && size(bel,2)==img.cols);
end

imagesc(bel); 
colormap(gray);
