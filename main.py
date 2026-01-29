"""Foosball IR break-beam simulator.

README
- This program simulates a 2D foosball goal with IR break-beam sensors to test
  speed estimation strategies under realistic shot angles, lateral offsets, and
  sensor latency/jitter.
- Batch experiments: python3 main.py --mode batch --n_shots 1000 --plots
- Single-shot visualization: python3 main.py --mode single --visualize
"""
from __future__ import annotations

from typing import Optional

import csv
import math
import numpy as np

from config import ExperimentConfig, SimulationConfig, Shot, parse_args
from estimators import FriendEstimator, TOFEstimator, build_ratio_mapping
from plotting import BatchMetrics, animate_single_shot, plot_batch
from simulation import ShotResult, sample_shot, simulate_shot, y_at_beam


def compute_percent_error(v_est: float, v_true: float) -> float:
    if math.isnan(v_est) or v_true == 0:
        return math.nan
    return (v_est - v_true) / v_true * 100.0


def summary_stats(errors: np.ndarray) -> dict[str, float]:
    clean = errors[~np.isnan(errors)]
    if clean.size == 0:
        return {
            "mean_abs": math.nan,
            "median_abs": math.nan,
            "p90": math.nan,
            "p95": math.nan,
            "worst": math.nan,
        }
    abs_err = np.abs(clean)
    return {
        "mean_abs": float(np.mean(abs_err)),
        "median_abs": float(np.median(abs_err)),
        "p90": float(np.percentile(abs_err, 90)),
        "p95": float(np.percentile(abs_err, 95)),
        "worst": float(np.max(abs_err)),
    }


def print_summary(label: str, stats: dict[str, float]) -> None:
    print(f"{label}:")
    print(f"  mean abs % error: {stats['mean_abs']:.2f}")
    print(f"  median abs % error: {stats['median_abs']:.2f}")
    print(f"  90th pct abs % error: {stats['p90']:.2f}")
    print(f"  95th pct abs % error: {stats['p95']:.2f}")
    print(f"  worst abs % error: {stats['worst']:.2f}")


def run_single(
    sim_config: SimulationConfig,
    exp_config: ExperimentConfig,
    ratio_mapping,
    poly_coeffs: Optional[list[float]],
) -> None:
    rng = np.random.default_rng(exp_config.seed)
    shot = sample_shot(
        rng,
        sim_config,
        exp_config.speed_min,
        exp_config.speed_max,
        exp_config.angle_min,
        exp_config.angle_max,
        exp_config.angle_sigma,
    )
    result = simulate_shot(shot, sim_config, rng, exp_config.poll_rate_hz)

    tof = TOFEstimator(beam1_id=1, beam2_id=2)
    friend = FriendEstimator(beam1_id=1, beam2_id=2, ratio_to_theta=ratio_mapping)

    tof_result = tof.estimate(result.events, sim_config)
    friend_result = friend.estimate(result.events, sim_config)

    print("Single-shot report")
    print(f"  true speed: {shot.speed:.3f} m/s")
    print(f"  true angle: {shot.theta:.3f} rad")
    print("  intervals (ideal):")
    for interval in result.intervals:
        print(f"    beam {interval.beam_id}: enter={interval.t_enter:.6f} exit={interval.t_exit:.6f}")
    print("  measured edges:")
    for event in result.events:
        print(f"    beam {event.beam_id} {event.edge} @ {event.time:.6f}")

    print("  estimates:")
    print(f"    TOF v_est={tof_result.v_est:.3f} m/s ({tof_result.reason or 'ok'})")
    print(
        f"    Friend v_est={friend_result.v_est:.3f} m/s theta_est={friend_result.details.get('theta_est', math.nan):.3f}"
        f" ({friend_result.reason or 'ok'})"
    )

    if exp_config.visualize:
        max_t = 0.0
        for times in result.edge_times.values():
            max_t = max(max_t, times[1])
        max_t = max(max_t, (sim_config.goal_depth - shot.x0) / max(shot.vx, 0.1))
        times = np.linspace(0.0, max_t, 200)
        positions = np.column_stack((shot.x0 + shot.vx * times, shot.y0 + shot.vy * times))
        beam_segments = [(b.ax, b.ay, b.bx, b.by) for b in sim_config.beams]
        est_speeds = {"TOF": tof_result.v_est, "Friend": friend_result.v_est}
        animate_single_shot(
            positions,
            times,
            beam_segments,
            sim_config.goal_width,
            sim_config.goal_depth,
            sim_config.ball_radius,
            shot.speed,
            est_speeds,
            result.edge_times,
        )


def run_batch(
    sim_config: SimulationConfig,
    exp_config: ExperimentConfig,
    ratio_mapping,
    poly_coeffs: Optional[list[float]],
) -> None:
    rng = np.random.default_rng(exp_config.seed)
    tof = TOFEstimator(beam1_id=1, beam2_id=2)
    friend = FriendEstimator(beam1_id=1, beam2_id=2, ratio_to_theta=ratio_mapping)

    angles: list[float] = []
    lateral: list[float] = []
    errors = {"tof": [], "friend": []}
    ratios: list[float] = []

    rows = []

    for _ in range(exp_config.n_shots):
        shot = sample_shot(
            rng,
            sim_config,
            exp_config.speed_min,
            exp_config.speed_max,
            exp_config.angle_min,
            exp_config.angle_max,
            exp_config.angle_sigma,
        )
        result = simulate_shot(shot, sim_config, rng, exp_config.poll_rate_hz)

        tof_result = tof.estimate(result.events, sim_config)
        friend_result = friend.estimate(result.events, sim_config)

        angles.append(shot.theta)
        y_beam = y_at_beam(shot, sim_config.beam_x1)
        lateral.append(y_beam if y_beam is not None else math.nan)

        errors["tof"].append(compute_percent_error(tof_result.v_est, shot.speed))
        errors["friend"].append(compute_percent_error(friend_result.v_est, shot.speed))
        ratios.append(friend_result.details.get("ratio", math.nan))

        if exp_config.export_csv:
            rows.append(
                {
                    "x0": shot.x0,
                    "y0": shot.y0,
                    "vx": shot.vx,
                    "vy": shot.vy,
                    "v_true": shot.speed,
                    "theta_true": shot.theta,
                    "tof_v_est": tof_result.v_est,
                    "friend_v_est": friend_result.v_est,
                    "ratio": friend_result.details.get("ratio", math.nan),
                }
            )

    errors_arr = {name: np.array(vals) for name, vals in errors.items()}
    ratios_arr = np.array(ratios)

    print("Batch summary")
    for name, errs in errors_arr.items():
        stats = summary_stats(errs)
        print_summary(name, stats)

    if exp_config.export_csv:
        with open(exp_config.export_csv, "w", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=rows[0].keys())
            writer.writeheader()
            writer.writerows(rows)
        print(f"Exported results to {exp_config.export_csv}")

    if exp_config.plots:
        metrics = BatchMetrics(
            angles=np.array(angles),
            lateral=np.array(lateral),
            errors=errors_arr,
            ratios=ratios_arr,
        )
        plot_batch(metrics)


def main() -> None:
    sim_config, exp_config, poly_coeffs = parse_args()
    ratio_mapping = build_ratio_mapping(exp_config.ratio_mapping, poly_coeffs)
    if exp_config.mode == "single":
        run_single(sim_config, exp_config, ratio_mapping, poly_coeffs)
    else:
        run_batch(sim_config, exp_config, ratio_mapping, poly_coeffs)


if __name__ == "__main__":
    main()
