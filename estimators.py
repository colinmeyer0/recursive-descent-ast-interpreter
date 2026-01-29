"""Estimators for speed from beam events."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Optional

import math

from sensor import Event, find_edge_time


@dataclass
class EstimateResult:
    name: str
    v_est: float
    details: dict
    reason: Optional[str] = None


class Estimator:
    name: str

    def estimate(self, events: list[Event], config) -> EstimateResult:
        raise NotImplementedError


class TOFEstimator(Estimator):
    """Two-beam time-of-flight estimator."""

    def __init__(self, beam1_id: int, beam2_id: int, edge: str = "fall") -> None:
        self.name = "tof"
        self.beam1_id = beam1_id
        self.beam2_id = beam2_id
        self.edge = edge

    def estimate(self, events: list[Event], config) -> EstimateResult:
        t1 = find_edge_time(events, self.beam1_id, self.edge)
        t2 = find_edge_time(events, self.beam2_id, self.edge)
        if t1 is None or t2 is None:
            return EstimateResult(self.name, math.nan, {}, reason="missing_edge")
        delta_t = t2 - t1
        if delta_t <= 0:
            return EstimateResult(self.name, math.nan, {"delta_t": delta_t}, reason="non_positive_delta_t")
        v_est = config.beam_spacing / delta_t
        return EstimateResult(self.name, v_est, {"delta_t": delta_t})


class FriendEstimator(Estimator):
    """Friend's ratio-based theory with a pluggable ratio->theta mapping."""

    def __init__(
        self,
        beam1_id: int,
        beam2_id: int,
        ratio_to_theta: Callable[[float], float],
        edge: str = "fall",
    ) -> None:
        self.name = "friend"
        self.beam1_id = beam1_id
        self.beam2_id = beam2_id
        self.edge = edge
        self.ratio_to_theta = ratio_to_theta

    def estimate(self, events: list[Event], config) -> EstimateResult:
        t_fall1 = find_edge_time(events, self.beam1_id, "fall")
        t_rise1 = find_edge_time(events, self.beam1_id, "rise")
        t_fall2 = find_edge_time(events, self.beam2_id, "fall")
        t_rise2 = find_edge_time(events, self.beam2_id, "rise")

        if None in (t_fall1, t_rise1, t_fall2, t_rise2):
            return EstimateResult(self.name, math.nan, {}, reason="missing_edge")

        t_block1 = t_rise1 - t_fall1
        t_block2 = t_rise2 - t_fall2
        delta_t = t_fall2 - t_fall1
        if delta_t <= 0:
            return EstimateResult(self.name, math.nan, {"delta_t": delta_t}, reason="non_positive_delta_t")

        if t_block2 == 0:
            return EstimateResult(self.name, math.nan, {"t_block2": t_block2}, reason="zero_block_time")

        r = t_block1 / t_block2
        theta_est = self.ratio_to_theta(r)
        v_est = (config.beam_spacing / delta_t) / math.cos(theta_est)

        details = {
            "t_block1": t_block1,
            "t_block2": t_block2,
            "delta_t": delta_t,
            "ratio": r,
            "theta_est": theta_est,
        }
        return EstimateResult(self.name, v_est, details)


def build_ratio_mapping(mode: str, poly_coeffs: Optional[list[float]] = None) -> Callable[[float], float]:
    if mode in ("default", "zero"):
        return lambda r: 0.0
    if mode == "poly":
        if not poly_coeffs:
            raise ValueError("poly_coeffs required for poly mapping")

        def mapping(r: float) -> float:
            total = 0.0
            for power, coef in enumerate(poly_coeffs):
                total += coef * (r ** power)
            return total

        return mapping
    raise ValueError(f"Unknown mapping mode: {mode}")
