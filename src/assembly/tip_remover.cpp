//
// Created by vout on 11/21/18.
//

#include "tip_remover.h"
#include "unitig_graph.h"

uint32_t RemoveTips(UnitigGraph &graph, uint32_t max_tip_len) {
  uint32_t num_removed = 0;
  for (uint32_t thre = 2; thre < max_tip_len; thre = std::min(thre * 2, max_tip_len)) {
#pragma omp parallel for reduction(+: num_removed)
    for (UnitigGraph::size_type i = 0; i < graph.size(); ++i) {
      auto adapter = graph.MakeVertexAdapter(i);
      if (adapter.length() >= thre) {
        continue;
      }
      if (adapter.forsure_standalone()) {
        adapter.set_to_delete();
        ++num_removed;
      } else {
        UnitigGraph::VertexAdapter nexts[4], prevs[4];
        int outd = graph.GetNextAdapters(adapter, nexts);
        int ind = graph.GetPrevAdapters(adapter, prevs);

        if (ind + outd == 0) {
          adapter.set_to_delete();
          ++num_removed;
        } else if (outd == 1 && ind == 0) {
          if (nexts[0].avg_depth() > 8 * adapter.avg_depth()) {
            adapter.set_to_delete();
            ++num_removed;
          }
        } else if (outd == 0 && ind == 1) {
          if (prevs[0].avg_depth() > 8 * adapter.avg_depth()) {
            adapter.set_to_delete();
            ++num_removed;
          }
        }
      }
    }

    graph.Refresh(false);
    if (thre >= max_tip_len) { break; }
  }
  return num_removed;
}
