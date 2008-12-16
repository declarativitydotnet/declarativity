function [x,y,z]=loaddata(filename,varargin)
%function [x,y]=loaddata(filename[,n])
%0,tcp,http,SF,181,5450,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,8,8,0.00,0.00,0.00,0.00,1.00,0.00,0.00,9,9,1.00,0.00,0.11,0.00,0.00,0.00,0.00,0.00,normal.

f=fopen(filename);
data=textscan(f, ['%n %s %s %s' repmat(' %n',1,37) '%s\n'],varargin{:});

k = size(data,2);
n = length(data{1});

x = horzcat(data{[1 5:k-1]});
y = ones(n,1)*nan;
z = ones(n,1)*nan;

labels = {'back' 2;
  'buffer_overflow' 3;
  'ftp_write' 4;
  'guess_passwd' 4;
  'imap' 4;
  'ipsweep' 1;
  'land' 2;
  'loadmodule' 3;
  'multihop' 4;
  'neptune' 2;
  'nmap' 1;
  'normal' 0;
  'perl' 3;
  'phf' 4;
  'pod' 2;
  'portsweep' 1;
  'rootkit' 3
  'satan' 1;
  'smurf' 2;
  'spy' 4;
  'teardrop' 2;
  'warezclient' 4;
  'warezmaster' 4};

for i=1:size(labels,1)
  found = strcmp(data{end}, [labels{i,1} '.']);
  y(found) = labels{i,2};
  z(found) = i;
end

% filter out unknown attack types
nans = isnan(y);
if any(nans)
  warning(sprintf('%d/%d unknown entries', sum(nans), length(y)));
  x=x(~nans,:);
  y=y(~nans);
end

% 0 normal
% 1 probe
% 2 denial of service (DOS)
% 3 user-to-root (U2R)
% 4 remote-to-local (R2L) 

% src_bytes: continuous.
% dst_bytes: continuous.
% land: symbolic.
% wrong_fragment: continuous.
% urgent: continuous.
% hot: continuous.
% num_failed_logins: continuous.
% logged_in: symbolic.
% num_compromised: continuous.
% root_shell: continuous.
% su_attempted: continuous.
% num_root: continuous.
% num_file_creations: continuous.
% num_shells: continuous.
% num_access_files: continuous.
% num_outbound_cmds: continuous.
% is_host_login: symbolic.
% is_guest_login: symbolic.
% count: continuous.
% srv_count: continuous.
% serror_rate: continuous.
% srv_serror_rate: continuous.
% rerror_rate: continuous.
% srv_rerror_rate: continuous.
% same_srv_rate: continuous.
% diff_srv_rate: continuous.
% srv_diff_host_rate: continuous.
% dst_host_count: continuous.
% dst_host_srv_count: continuous.
% dst_host_same_srv_rate: continuous.
% dst_host_diff_srv_rate: continuous.
% dst_host_same_src_port_rate: continuous.
% dst_host_srv_diff_host_rate: continuous.
% dst_host_serror_rate: continuous.
% dst_host_srv_serror_rate: continuous.
% dst_host_rerror_rate: continuous.
% dst_host_srv_rerror_rate: continuous.
