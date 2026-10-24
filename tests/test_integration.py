import json
import os
import signal
import socket
import subprocess
import tempfile
import time
import unittest
from pathlib import Path

BINARY = Path(os.environ.get("RAILDIAG_BINARY", Path(__file__).resolve().parents[1] / "build" / "raildiag-gateway"))


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return int(sock.getsockname()[1])


class GatewayIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not BINARY.exists():
            raise RuntimeError(f"Gateway binary not found: {BINARY}. Build with CMake first.")
        cls.tempdir = tempfile.TemporaryDirectory()
        cls.port = free_port()
        cls.log_path = Path(cls.tempdir.name) / "diagnostics.csv"
        cls.proc = subprocess.Popen(
            [str(BINARY), "--bind", "127.0.0.1", "--port", str(cls.port), "--log", str(cls.log_path)],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        deadline = time.time() + 5
        while time.time() < deadline:
            if cls.proc.poll() is not None:
                out, err = cls.proc.communicate()
                raise RuntimeError(f"Gateway exited early. stdout={out} stderr={err}")
            try:
                with socket.create_connection(("127.0.0.1", cls.port), timeout=0.2):
                    break
            except OSError:
                time.sleep(0.05)
        else:
            cls.proc.terminate()
            raise RuntimeError("Gateway did not start listening within 5 seconds")

    @classmethod
    def tearDownClass(cls):
        if cls.proc.poll() is None:
            cls.proc.terminate()
            try:
                cls.proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                cls.proc.kill()
                cls.proc.wait(timeout=3)
        if cls.proc.stdout:
            cls.proc.stdout.close()
        if cls.proc.stderr:
            cls.proc.stderr.close()
        cls.tempdir.cleanup()

    def request(self, frame: str) -> dict:
        with socket.create_connection(("127.0.0.1", self.port), timeout=2) as sock:
            sock.sendall((frame + "\n").encode("ascii"))
            stream = sock.makefile("r", encoding="utf-8")
            line = stream.readline()
            if not line:
                return {}
            return json.loads(line)

    def test_normal_sample_is_ok(self):
        result = self.request("RDG1|normal_ecu|1|62.5|24.1|1.8|OK")
        self.assertEqual(result["state"], "OK")

    def test_overtemperature_is_fault(self):
        result = self.request("RDG1|hot_ecu|1|95.0|24.0|1.0|OK")
        self.assertEqual(result["state"], "FAULT")
        self.assertIn("OVERTEMPERATURE", result["events"])

    def test_voltage_warning(self):
        result = self.request("RDG1|voltage_ecu|1|60.0|20.0|1.0|OK")
        self.assertEqual(result["state"], "WARN")

    def test_malformed_sample_is_rejected(self):
        result = self.request("NOT-RDG|sensor|1|60|24|1|OK")
        self.assertEqual(result["error"], "invalid_frame")

    def test_non_finite_sample_is_rejected(self):
        result = self.request("RDG1|nan_ecu|1|nan|24|1|OK")
        self.assertEqual(result["error"], "invalid_frame")

    def test_replayed_sequence_is_rejected(self):
        first = self.request("RDG1|replay_ecu|8|60|24|1|OK")
        second = self.request("RDG1|replay_ecu|8|60|24|1|OK")
        self.assertEqual(first["state"], "OK")
        self.assertEqual(second["error"], "replayed_or_out_of_order_sequence")

    def test_accepted_sample_is_logged(self):
        self.request("RDG1|log_ecu|1|55|24|1|OK")
        content = self.log_path.read_text(encoding="utf-8")
        self.assertIn("log_ecu,1,55", content)

    def test_warning_sample_is_warning(self):
        result = self.request("RDG1|warn_ecu|1|78|20.5|1|WARN")
        self.assertEqual(result["state"], "WARN")

    def test_multiple_clients_can_be_served(self):
        import concurrent.futures
        frames = [f"RDG1|parallel_{index}|1|60|24|1|OK" for index in range(8)]
        with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
            results = list(pool.map(self.request, frames))
        self.assertEqual(len(results), 8)
        self.assertTrue(all(result["state"] == "OK" for result in results))

    def test_compound_fault_and_warning_events_preserved(self):
        result = self.request("RDG1|compound_ecu|1|95.0|20.0|1.5|OK")
        self.assertEqual(result["state"], "FAULT")
        self.assertIn("OVERTEMPERATURE", result["events"])
        self.assertIn("UNDERVOLTAGE_WARNING", result["events"])

    def test_frame_too_large_is_rejected(self):
        large_payload = "RDG1|large_ecu|1|" + ("9" * 600) + "|24.0|1.0|OK"
        with socket.create_connection(("127.0.0.1", self.port), timeout=2) as sock:
            sock.sendall((large_payload + "\n").encode("ascii"))
            stream = sock.makefile("r", encoding="utf-8")
            response = stream.readline()
            self.assertTrue(len(response) > 0)
            data = json.loads(response)
            self.assertEqual(data.get("error"), "frame_too_large")

    def test_orderly_sigterm_shutdown(self):
        temp_port = free_port()
        log_file = Path(self.tempdir.name) / "sigterm_diag.csv"
        proc = subprocess.Popen(
            [str(BINARY), "--bind", "127.0.0.1", "--port", str(temp_port), "--log", str(log_file)],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        time.sleep(0.3)
        proc.send_signal(signal.SIGTERM)
        exit_code = proc.wait(timeout=3)
        if proc.stdout:
            proc.stdout.close()
        if proc.stderr:
            proc.stderr.close()
        self.assertEqual(exit_code, 0)


if __name__ == "__main__":
    unittest.main()
