#!/usr/bin/env perl

use Time::Local;
use strict;

# assign:

#2009-05-06 04:06:07,879 INFO org.apache.hadoop.mapred.JobTracker: Adding task 'attempt_200905060355_0001_m_000000_0' to tip task_200905060355_0001_m_000000, for tracker 'tracker_ip-10-251-31-19.ec2.internal:localhost.localdomain/127.0.0.1:52651'


## Completion log:
#2009-05-06 04:07:35,856 INFO org.apache.hadoop.mapred.JobInProgress: Task 'attempt_200905060355_0001_m_000000_0' has completed task_200905060355_0001_m_000000 successfully.

# choice
# 2009-05-06 04:06:08,222 INFO org.apache.hadoop.mapred.JobInProgress: Choosing data-local task task_200905060355_0001_m_000011

#job start
#2009-05-09 02:11:11,381 INFO org.apache.hadoop.mapred.JobInProgress: Split info for job:job_200905090202_0001

#job end
#2009-05-09 02:12:39,533 INFO org.apache.hadoop.mapred.JobInProgress: Job job_200905090202_0001 has completed successfully.

# progress update
#2009-09-29 20:19:50,668 INFO org.apache.hadoop.mapred.JobInProgress: JobInProgress job_200909292018_0001 map progress 0.6547225 reduce progress 0.15

ddl();

my %starts;
my %jobstart;
my %summary;
my %types; 
my %twc;
my %done;

while (<>) {
    chomp;


    if (/(\S+) (\S+) INFO org.apache.hadoop.mapred.JobInProgress: Split info for job:(\S+)/) {
        my ($date, $time, $job) = ($1, $2, $3);
        $jobstart{$job}{'start'} = epoch($date, $time);
    } elsif (/(\S+) (\d+:\d+:\d+),\d+ INFO org.apache.hadoop.mapred.JobInProgress: Task ('[^']+') has completed (\S+) successfully/) {
        my ($date, $time, $attempt, $task) = ($1, $2, $3, $4);
        my $diff = epoch($date, $time) - $starts{$attempt};
        my $job = get_job($task);
        
        #print "$totaltime\t$job\t$task\t$types{$task}\t$diff\n";
        push @{$summary{$job}{$task}}, [epoch($date, $time), $job, $task, $types{$task}->[0], $types{$task}->[1], $diff];

    } elsif (/(\S+) (\S+) INFO org.apache.hadoop.mapred.JobTracker: Adding task ('[^']+') to tip (\S+), for tracker '[^']+'/) {
        my ($date, $time, $attempt, $task) = ($1, $2, $3, $4);
        $starts{$attempt} = epoch($date, $time);
    } elsif (/(\S+) (\S+) INFO org.apache.hadoop.mapred.JobInProgress: Choosing ([^-]+)-local task (\S+)(.*)/) {
        my ($date,$time,$type,$task,$spec) = ($1, $2, $3, $4, $5);

        $type =~ s/^a //;
        my $t;
        if ($spec) {
            $t = "speculation";
        } else {
            $t = "normal";
        }
        $types{$task} = [$type, $t];
    } elsif (/(\S+) (\S+) INFO org.apache.hadoop.mapred.JobInProgress: Job (\S+) has completed successfully./) {
        my ($date, $time, $job) = ($1, $2, $3);
        $jobstart{$job}{'end'} = epoch($date, $time);


    } elsif (/(\S+) (\d+:\d+:\d+),\d+ INFO org.apache.hadoop.mapred.JobInProgress: JobInProgress (\S+) map progress (\d+\.\d+) reduce progress (\d+\.\d+)/) {
        my ($date, $time, $job, $mp, $rp) = ($1, $2, $3, $4, $5);
        my $epoch = epoch($date, $time);
        my $diff = epoch($date, $time) - $jobstart{$job}{'start'};
        #print STDERR "JOB start " . $jobstart{$job}{'start'} . "\n";
        if (!defined($jobstart{$job}{'start'})) {
            print STDERR "#input from undefined job: *$_*\n";
            next;
        }
        
        if (!defined $twc{$job}{'sec'}) {
            #print STDERR "INIT!\n";
            $twc{$job}{'sec'} = 0;
            $twc{$job}{'mp'} = 0;
            $twc{$job}{'rp'} = 0;
        }
        #print STDERR "diff is $diff\n";
        for (my $i = $twc{$job}{'sec'} + 1; $i < $diff && $diff < 10000; $i++) {
            print STDERR "$job\t$i\t$twc{$job}{'mp'}\t$twc{$job}{'rp'}\n";
        }
        $twc{$job}{'sec'} = $diff;
        $twc{$job}{'mp'} = $mp;
        $twc{$job}{'rp'} = $rp;

        my $comb = $job.$diff;

        if (!$done{$comb}) {
            print STDERR "$job\t$diff\t$mp\t$rp\n";
            $done{$comb} = 1;
        }
        
    } elsif (/<\s*([^>]+)\s*>/) {
        my @stuff = split(/\s*,\s*/, $1);

        if ($#stuff == 11) {
            #print "JOB: $stuff[0]\t$stuff[9]\t$stuff[10]\t$stuff[11]\n";
            my $job = $stuff[0];

            if ($stuff[11] eq 'PREP') {
                $jobstart{$job}{'start'} = $stuff[9];
            } elsif ($stuff[11] eq 'SUCCEEDED') {
                $jobstart{$job}{'end'} = $stuff[10];
            }
        } elsif($#stuff == 8) {
            #print "TASK: $stuff[2]\t$stuff[3]\t$stuff[4]\t$stuff[5]\t$stuff[7]\t$stuff[8]\n";

            if (($stuff[4] eq 'SUCCEEDED') || ($stuff[4] eq 'COMMIT_PENDING')) {
                my $task = get_task($stuff[2]);
                my $job = get_job($task);


                #print "TASK: $task\n";

                my $start_dt = $stuff[7];
                my $end_dt = $stuff[8];

                my $diff = $end_dt - $start_dt;

                push @{$summary{$job}{$task}}, [$end_dt, $job, $task, $types{$task}->[0], $types{$task}->[1], $diff];
            }
        }

    } else {
        #print STDERR "UNK: $_\n";
    }
}

print "\n";

foreach (keys %jobstart) {
    my $diff = $jobstart{$_}{'end'} - $jobstart{$_}{'start'};
    #print "$_\t$diff\n";
    table("job", [$_, $jobstart{$_}{'start'}, $diff]);
}

foreach my $job (sort keys %summary) {
    foreach my $task (sort keys %{$summary{$job}}) {
        my @attempts = @{$summary{$job}{$task}};
        if ($#attempts > 0) {
            print "multiple attempts!\n";
        }
        foreach my $attempt (@attempts) {
            #print "$job\t$task\t".join("\t", @{$attempt})."\n";
            table("jt", $attempt);
        }
    }
}

statement();


sub table {
    my ($tab, $att) = @_;

    print "insert into $tab values(".join(",",map("\'$_\'", @{$att})).");\n";
}

sub ddl {
    my $char = "string";    
    my $int = "int";
    print "drop table jt; drop table job;\n";
    print "create table jt (date $char, job $char, task $char, locality $char, type $char, task_time $int);\n";
    
    print "create table job (job $char, start_time $char, runtime $char);\n";
}

sub get_job {
    my ($task) = @_;
    my $job;
    if ($task =~ /(task|attempt)_(\d+_\d+)/) {
        $job = $2;
    } else {
        print "NO MATCH! $task\n";
    }
    return $job;
}

sub get_task {
    my ($str) = @_;
    my $task;
    if ($str =~ /attempt_(\d+\_\d+\_\w\_)/) {
        return "task_".$1;
    }
}

sub epoch {
    my ($date, $time) = @_;
    # $time = timelocal($sec,$min,$hour,$mday,$mon,$year);
    if ($date =~ /(\d{4,})-(\d{2,})-(\d{2,})/) {
        my ($year, $mon, $day) = ($1, $2, $3);
        if ($time =~ /(\d{2,}):(\d{2,}):(\d{2,})/) {
            my ($hh,$mi,$ss) = ($1, $2, $3);
            my $time = timelocal($ss,$mi,$hh,$day,$mon-1,$year-1900);
            return $time;
        }
    }
}


sub statement {
    my $SQL=<<END;
select job, runtime
    from job;

select '_______________________';

select job, count(distinct task)
    from jt
        group by job;

select '_______________________';

select job, avg(task_time),count(*)
    from jt
        where task like '%_m_%'
            group by job;

select '_______________________';
select job, locality, type, avg(task_time),count(*)
    from jt
        where task like '%_m_%'
            group by job, locality, type;

select '_______________________';

drop view ncdf3;
create view ncdf3 as
select job, di * 10 di, type, count(*) cnt from (
select job.job job, task, (jt.date - job.start_time) / 10  di, case when jt.task like '%_m_%' then 'MAP' when jt.task like '%_r_%' then 'REDUCE' else 'OTHER' end type
    from job, jt
        where job.job = 'job_' || jt.job
) 
    group by job, di, type;

drop view ncdf;
create view ncdf as
select job, di, type, count(*) cnt from (
select job.job job, task, (jt.date - job.start_time) di, case when jt.task like '%_m_%' then 'MAP' when jt.task like '%_r_%' then 'REDUCE' else 'OTHER' end type
    from job, jt
        where job.job = 'job_' || jt.job
) 
    group by job, di, type;




drop view totals;
create view totals as
    select job, type, sum(cnt)+0.0 cnt,max(di) maxdi 
        from ncdf
            group by job, type;


select j, t, di, (bcnt / t.cnt) from (
select a.job j, a.di di,a.type t, sum(b.cnt) + 0.0  bcnt
    from ncdf a, ncdf b
        where a.type = b.type
        and a.job = b.job
        and a.di > b.di
            group by a.job, a.di, a.type
) a, totals t
    where t.job = a.j
    and t.type = a.t
        order by j, t, di, 3;

/*

select '______________________________';


drop view farg;
create view farg as 
select j, t, di, bcnt, t.cnt tcnt from (
select a.job j, a.di di,a.type t, sum(b.cnt) + 0.0  bcnt
    from ncdf a, ncdf b
        where a.type = b.type
        and a.job = b.job
        and a.di > b.di
            group by a.job, a.di, a.type
) a, totals t
    where t.job = a.j
    and t.type = a.t
        order by j, t, di, 3;


select t, di, avg(bcnt/tcnt) from farg
    group by t, di
        order by t,di;

select job, type, count(*) from (
select job.job job, case when jt.task like 
'%_m_%' then 'MAP' when jt.task like '%_r_%' then 'REDUCE' else 'OTHER' end type
    from job, jt
        where job.job = 'job_' || jt.job
)
group by job,type;


drop view v_interm;

create view v_interm as
    select job, sum(task_time) a, sum(task_time * task_time) a2, count(*) cnt
        from jt
            where task like '%_m_%'
                group by job;


select job,cnt,  avg, sqrt((agg - (avg * avg)) / cnt) std from (
    select job, a / cnt avg, a2 agg, cnt
        from v_interm
);
*/


END
    print "$SQL\n";
}
