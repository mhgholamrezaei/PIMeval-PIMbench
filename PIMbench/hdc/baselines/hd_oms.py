import argparse
import math
import time
import torch
import model
import utils
import csv

def load_dataset_and_quantize(ref_fname, query_fname, n_lv):
    """Load datasets and perform quantization."""
    print("[INFO] Loading datasets")
    ds_ref, ds_query = utils.load_dataset(name="OMS_iPRG_demo", path=[ref_fname, query_fname])

    print("[INFO] Quantizing datasets")
    ds_ref["levels"][ds_ref["levels"] == -1] = 0
    ds_query["levels"][ds_query["levels"] == -1] = 0

    ds_ref_levels_quantized = model.min_max_quantize(
        torch.tensor(ds_ref["levels"], dtype=torch.float32), int(math.log2(n_lv) - 1)
    )
    ds_query_levels_quantized = model.min_max_quantize(
        torch.tensor(ds_query["levels"], dtype=torch.float32), int(math.log2(n_lv) - 1)
    )

    ds_ref_idxs = torch.tensor(ds_ref["idxs"], dtype=torch.float32)
    ds_query_idxs = torch.tensor(ds_query["idxs"], dtype=torch.float32)

    return ds_ref_levels_quantized, ds_ref_idxs, ds_query_levels_quantized, ds_query_idxs

def encode_data(hdc_model, ds_levels_quantized, ds_idxs, n_samples):
    """Perform HDC encoding."""
    print(f"[INFO] Encoding {n_samples} samples")
    encoded_data = hdc_model.encode(
        {"lv": ds_levels_quantized[:n_samples], "idx": ds_idxs[:n_samples]}, dense=False
    )
    return encoded_data

def write_tensor_to_csv(tensor, filename):
    if not isinstance(tensor, torch.Tensor):
        raise TypeError("Input must be a torch.Tensor")

    dim1, dim2 = tensor.shape

    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write dimensions
        writer.writerow([dim1, dim2])
        # Write data rows
        for row in tensor.tolist():
            writer.writerow(row)

def search_database(hdc_model, query_enc, ref_enc, device):
    """Perform database search."""
    print("[INFO] Searching database")

    # Move data to the appropriate device
    query_enc = query_enc.to(device).float()
    ref_enc = ref_enc.to(device).float()

    start_time = time.time()
    dist = torch.matmul(query_enc, ref_enc.T)
    pred = dist.argmax(dim=-1)

    end_time = time.time()
    print(f"[INFO] Execution time: {end_time - start_time:.6f} seconds")
    return pred

def main(args):
    print("[INFO] Starting main function")

    # Set device
    device = torch.device("cuda" if args.cuda and torch.cuda.is_available() else "cpu")
    print(f"[INFO] Using device: {device}")

    # File paths
    ref_fname = f"../Dataset/human_yeast_targetdecoy_vec_{args.dim_spectra}.charge{args.charge}.npz"
    query_fname = f"../Dataset/iPRG2012_vec_{args.dim_spectra}.charge{args.charge}.npz"

    # HDC model setup
    print("[INFO] Initializing HDC model")
    hdc_model = model.HDC_ID_LV(
        n_class=args.n_class,
        n_lv=args.n_lv,
        n_id=args.dim_spectra,
        n_dim=args.n_dim,
        method_id_lv="cyclic",
        binary=True,
    )

    # Load and quantize datasets
    ds_ref_levels_quantized, ds_ref_idxs, ds_query_levels_quantized, ds_query_idxs = load_dataset_and_quantize(
        ref_fname, query_fname, args.n_lv
    )

    # Encoding
    ref_enc = encode_data(hdc_model, ds_ref_levels_quantized, ds_ref_idxs, args.n_test)
    query_enc = encode_data(hdc_model, ds_query_levels_quantized, ds_query_idxs, args.n_query)

    # Write to binary file
    write_tensor_to_csv(ref_enc, args.output_ref_file)
    write_tensor_to_csv(query_enc, args.output_query_file)

    # Database search
    search_database(hdc_model, query_enc, ref_enc, device)

if __name__ == "__main__":
    print("[INFO] Parsing command line arguments")
    parser = argparse.ArgumentParser(description="HDC-based database search")
    parser.add_argument("-d", "--dim_spectra", type=int, default=34976, help="Dimension of spectra")
    parser.add_argument("-c", "--charge", type=int, default=2, help="Charge state")
    parser.add_argument("-nc", "--n_class", type=int, default=100, help="Number of classes")
    parser.add_argument("-nlv", "--n_lv", type=int, default=64, help="Quantization levels")
    parser.add_argument("-nd", "--n_dim", type=int, default=2048, help="HV dimension")
    parser.add_argument("-nt", "--n_test", type=int, default=100, help="Number of test samples")
    parser.add_argument("-nq", "--n_query", type=int, default=5, help="Number of query samples")
    parser.add_argument("--cuda", action="store_true", help="Enable GPU support for PyTorch")
    parser.add_argument("--output_ref_file", type=str, default="../PIM/ref.csv", help="Output CSV file for the reference encoding")
    parser.add_argument("--output_query_file", type=str, default="../PIM/query.csv", help="Output CSV file for the query encoding")
    args = parser.parse_args()

    print("[INFO] Starting the process")
    main(args)
