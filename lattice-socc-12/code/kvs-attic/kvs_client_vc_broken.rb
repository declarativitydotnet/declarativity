# XXX: This is broken because we need special merge rules to correctly accept
# kvputs from clients. The issue is that on kvput, the client submits the VC of
# the version _being updated_, not the VC that should be assigned to the new
# version of the datum. Hence we can't use the normal merge rules: we want to
# replace the server's previous value iff old.vc <= new.vc; otherwise, we want
# to merge the two values. This seems clumsy to implement, so the current
# approach is just to use a normal KVS and have clients increment VCs
# themselves. This means that the VCs are O(number of clients) rather than
# O(number of servers), which is undesirable in practice.
#
# Design notes:
#
# Inspired by Dynamo. Each key is associated with a <vector-clock, value>
# pair. A read returns this pair.
#
# To issue a write, a client must supply a <vector-clock, value> pair. The
# vector-clock represents the most recent version of this key read by the
# client, if any (if the client is doing a "blind write", they can use the empty
# clock). The write is received by a replica, which _coordinates_ applying the
# write to the rest of the system. First, the coordinator increments its own
# position in the client-submitted vector clock. It then merges the resulting
# <vector-clock, value> pair into its own database, according to this logic:
# versions are preferred according to the partial order over VCs, and for
# incomparable VCs (concurrent versions) a user-supplied merge method is
# invoked; the resulting merged value is assigned the merged VC of the two input
# values. Since we do conflict resolution at the server side, each replica
# stores at most one <vector-clock, value> associated with any key (unlike in
# Dynamo).
#
# Writes are propagated between replicas via the same logic, except that a
# replica accepting a write from another replica does not bump its own position
# in the write's vector clock. Write progagation could either be done actively
# (coordinator doesn't return success to the client until W replicas have been
# written) or passively (replicas perioidically merge their databases).
#
# Note that we maintain a vector clock at each node and increment it for _every_
# kvput; it is this clock that we use to stamp inbound kvputs. An alternative
# approach would be to just increment the previous logical clock for the
# originating node found in the kvput's VC. Unclear which approach is better;
# the current approach provides a hint about cross-key causality.
class VectorClockKvsReplica < MergeMapKvsReplica
  state do
    lmap :my_vc
    lmap :next_vc
  end

  bootstrap do
    my_vc <= { ip_port => Bud::MaxLattice.new(0) }
  end

  bloom :write do
    # Increment local VC on every kvput
    next_vc <= my_vc
    next_vc <= kvput { {ip_port => my_vc.at(ip_port) + 1} }
    my_vc <+ next_vc

    # On every kvput, merge incremented VC with kvput's VC
    kv_store <= kvput {|m| {m.key => m.value.apply_fst(:merge, next_vc)}}
  end
end
