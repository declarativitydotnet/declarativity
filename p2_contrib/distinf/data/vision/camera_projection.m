function q = camera_projection(calib, p)

q = calib.KK * [calib.R calib.T] * [p; ones(1, size(p,2))];
q = q(1:2,:) ./ repmat(q(3,:), 2, 1);
