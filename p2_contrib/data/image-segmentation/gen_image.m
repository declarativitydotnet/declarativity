function img=gen_image(m, n)
img.rows = m;
img.cols = n;

img.k = 2;
img.mu = [147, 150];
img.sigma = [2,1];

for i = 1:img.k
   img.noise{i} = sqrt(img.sigma(i)) * randn(img.rows, img.cols) + ...
      img.mu(i);
end

[U, V] = meshgrid(1:img.cols, 1:img.rows);
img.mask = sqrt((U-img.cols/2).^2+(V-img.rows/2).^2) < ...
   ( (img.rows+img.cols)/8 );
img.mask_bar = 1-img.mask;

img.data = img.mask .* img.noise{1} + img.mask_bar .* img.noise{2};

imagesc(img.data);
colormap(gray);
