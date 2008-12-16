function error = rms(belief, img)
assert(size(belief,2)==img.rows*img.cols);

truth = 1-double(img.mask(:))';

error = sqrt(mean((belief-repmat(truth,size(belief,1),1)).^2,2));
