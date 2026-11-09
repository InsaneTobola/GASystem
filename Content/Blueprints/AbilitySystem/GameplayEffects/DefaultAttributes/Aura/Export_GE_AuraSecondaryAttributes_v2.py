# Copyright (c) OpenAI. All rights reserved.
# Unreal Engine 5.7 Editor Python script.
# Read-only export of the Blueprint Class Default Object for GE_AuraSecondaryAttributes.
# The asset loaded by EditorAssetLibrary is a UBlueprint. Its GameplayEffect
# configuration lives on the generated class default object (CDO), not on the
# UBlueprint asset itself.

import os
import datetime
import traceback

import unreal


# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------

ASSET_PATH = (
    "/Game/Blueprints/AbilitySystem/GameplayEffects/"
    "DefaultAttributes/Aura/GE_AuraSecondaryAttributes"
)
EXPECTED_MODIFIER_COUNT = 20
REPORT_FOLDER_NAME = "Reports"
REPORT_FILE_NAME = "GE_AuraSecondaryAttributes_Report.txt"


# -----------------------------------------------------------------------------
# Safe reflection helpers
# -----------------------------------------------------------------------------


def read_property(obj, property_name):
    """Return (success, value, error text) for a reflected editor property."""
    if obj is None:
        return False, None, "Object is None"

    try:
        return True, obj.get_editor_property(property_name), ""
    except Exception as exc:
        return False, None, str(exc)


def get_prop(obj, property_name, default=None):
    ok, value, _ = read_property(obj, property_name)
    return value if ok else default


def object_path(obj):
    if obj is None:
        return "None"

    for method_name in ("get_path_name", "get_full_name", "get_name"):
        try:
            method = getattr(obj, method_name, None)
            if callable(method):
                value = method()
                if value:
                    return str(value)
        except Exception:
            pass

    return str(obj)


def enum_text(value):
    return "None / unavailable" if value is None else str(value)


def scalar_text(value):
    if value is None:
        return "None / unavailable"
    try:
        return "{:.6g}".format(float(value))
    except Exception:
        return str(value)


def curve_row_report(handle, indent="    "):
    if handle is None:
        return [indent + "Curve Table Row: None"]

    curve_table = get_prop(handle, "curve_table")
    row_name = get_prop(handle, "row_name")

    if curve_table is None and row_name is None:
        return [indent + "Curve Table Row: None"]

    return [
        indent + "Curve Table: " + object_path(curve_table),
        indent + "Row Name: " + str(row_name if row_name is not None else "None"),
    ]


def scalable_float_report(value, indent="    "):
    if value is None:
        return [indent + "Value: None / unavailable"]

    raw_value = get_prop(value, "value")
    curve_handle = get_prop(value, "curve")
    registry_type = get_prop(value, "registry_type")

    lines = [indent + "Raw Value: " + scalar_text(raw_value)]
    lines.extend(curve_row_report(curve_handle, indent + "  "))
    if registry_type is not None:
        lines.append(indent + "Data Registry Type: " + str(registry_type))
    return lines


def attribute_report(attribute):
    if attribute is None:
        return "None / unavailable"

    # GameplayAttribute string output contains the attribute name and owning set.
    # attribute_name is also queried separately when exposed by the Python API.
    attribute_name = get_prop(attribute, "attribute_name")
    raw_text = str(attribute)
    if attribute_name:
        return "{} | {}".format(attribute_name, raw_text)
    return raw_text


def tag_container_report(container):
    if container is None:
        return "None / unavailable"

    tags = get_prop(container, "gameplay_tags")
    if tags is None:
        # String conversion is a useful fallback for UE versions exposing only
        # the container's textual representation.
        return str(container)
    if len(tags) == 0:
        return "Empty"
    return ", ".join(str(tag) for tag in tags)


def tag_requirements_report(requirements, heading, indent="  "):
    lines = [indent + heading + ":"]
    if requirements is None:
        lines.append(indent + "  None / unavailable")
        return lines

    required = get_prop(requirements, "require_tags")
    ignored = get_prop(requirements, "ignore_tags")
    query = get_prop(requirements, "tag_query")

    lines.append(indent + "  Require Tags: " + tag_container_report(required))
    lines.append(indent + "  Ignore Tags: " + tag_container_report(ignored))
    lines.append(indent + "  Tag Query: " + (str(query) if query is not None else "None / unavailable"))
    return lines


def capture_definition_report(capture, indent="    "):
    if capture is None:
        return [indent + "Backing Attribute Capture: None / unavailable"]

    attribute = get_prop(capture, "attribute_to_capture")
    source = get_prop(capture, "attribute_source")
    snapshot = get_prop(capture, "snapshot")

    return [
        indent + "Backing Attribute: " + attribute_report(attribute),
        indent + "Attribute Source: " + enum_text(source),
        indent + "Snapshot: " + str(snapshot),
    ]


def attribute_based_float_report(value, indent="    "):
    lines = [indent + "Attribute-Based Magnitude:"]
    if value is None:
        lines.append(indent + "  None / unavailable")
        return lines

    for property_name, label in (
        ("coefficient", "Coefficient"),
        ("pre_multiply_additive_value", "Pre-Multiply Additive Value"),
        ("post_multiply_additive_value", "Post-Multiply Additive Value"),
    ):
        lines.append(indent + "  " + label + ":")
        lines.extend(scalable_float_report(get_prop(value, property_name), indent + "    "))

    lines.extend(capture_definition_report(get_prop(value, "backing_attribute"), indent + "  "))
    lines.append(
        indent + "  Attribute Calculation Type: "
        + enum_text(get_prop(value, "attribute_calculation_type"))
    )
    lines.append(indent + "  Attribute Curve:")
    lines.extend(curve_row_report(get_prop(value, "attribute_curve"), indent + "    "))
    lines.append(
        indent + "  Source Tag Filter: "
        + tag_container_report(get_prop(value, "source_tag_filter"))
    )
    lines.append(
        indent + "  Target Tag Filter: "
        + tag_container_report(get_prop(value, "target_tag_filter"))
    )
    return lines


def custom_magnitude_report(value, indent="    "):
    lines = [indent + "Custom Calculation Magnitude:"]
    if value is None:
        lines.append(indent + "  None / unavailable")
        return lines

    calculation_class = get_prop(value, "calculation_class_magnitude")
    lines.append(indent + "  Calculation Class: " + object_path(calculation_class))

    for property_name, label in (
        ("coefficient", "Coefficient"),
        ("pre_multiply_additive_value", "Pre-Multiply Additive Value"),
        ("post_multiply_additive_value", "Post-Multiply Additive Value"),
    ):
        lines.append(indent + "  " + label + ":")
        lines.extend(scalable_float_report(get_prop(value, property_name), indent + "    "))

    lines.append(indent + "  Final Lookup Curve:")
    lines.extend(curve_row_report(get_prop(value, "final_lookup_curve"), indent + "    "))
    return lines


def set_by_caller_report(value, indent="    "):
    lines = [indent + "Set-By-Caller Magnitude:"]
    if value is None:
        lines.append(indent + "  None / unavailable")
        return lines

    # SetByCallerFloat property naming varies slightly in the exposed API;
    # try the known UE property names, then retain the full raw struct string.
    found_any = False
    for property_name, label in (
        ("data_tag", "Data Tag"),
        ("data_name", "Data Name"),
        ("default_value", "Default Value"),
    ):
        ok, field_value, _ = read_property(value, property_name)
        if ok:
            lines.append(indent + "  " + label + ": " + str(field_value))
            found_any = True

    if not found_any:
        lines.append(indent + "  Raw Value: " + str(value))
    return lines


def gameplay_effect_magnitude_report(value, indent="    "):
    """Describe FGameplayEffectModifierMagnitude (used for GE durations)."""
    lines = [indent + "Magnitude Calculation Type: " + enum_text(
        get_prop(value, "magnitude_calculation_type") if value is not None else None
    )]
    if value is None:
        lines.append(indent + "Magnitude: None / unavailable")
        return lines

    calculation_type = get_prop(value, "magnitude_calculation_type")
    calculation_text = str(calculation_type).upper() if calculation_type is not None else ""

    if "SCALABLE" in calculation_text:
        lines.append(indent + "Scalable Float:")
        lines.extend(scalable_float_report(
            get_prop(value, "scalable_float_magnitude"), indent + "  "
        ))
    elif "ATTRIBUTE_BASED" in calculation_text:
        lines.extend(attribute_based_float_report(
            get_prop(value, "attribute_based_magnitude"), indent + "  "
        ))
    elif "CUSTOM" in calculation_text:
        lines.extend(custom_magnitude_report(
            get_prop(value, "custom_magnitude"), indent + "  "
        ))
    elif "SET_BY_CALLER" in calculation_text or "SET BY CALLER" in calculation_text:
        lines.extend(set_by_caller_report(
            get_prop(value, "set_by_caller_magnitude"), indent + "  "
        ))
    else:
        lines.append(indent + "Raw Magnitude Struct: " + str(value))
    return lines


def modifier_report(modifier, index):
    lines = ["=" * 96, "MODIFIER INDEX [{}]".format(index), "=" * 96]

    attribute = get_prop(modifier, "attribute")
    operation = get_prop(modifier, "modifier_op")
    magnitude = get_prop(modifier, "modifier_magnitude")

    lines.append("Attribute: " + attribute_report(attribute))
    lines.append("Modifier Operation: " + enum_text(operation))

    if magnitude is None:
        lines.append("Magnitude: None / unavailable")
    else:
        calculation_type = get_prop(magnitude, "magnitude_calculation_type")
        lines.append("Magnitude Calculation Type: " + enum_text(calculation_type))
        calculation_text = str(calculation_type).upper() if calculation_type is not None else ""

        if "ATTRIBUTE_BASED" in calculation_text:
            lines.extend(attribute_based_float_report(
                get_prop(magnitude, "attribute_based_magnitude"), "  "
            ))
        elif "CUSTOM" in calculation_text:
            lines.extend(custom_magnitude_report(
                get_prop(magnitude, "custom_magnitude"), "  "
            ))
        elif "SCALABLE" in calculation_text:
            lines.append("  Scalable Float Magnitude:")
            lines.extend(scalable_float_report(
                get_prop(magnitude, "scalable_float_magnitude"), "    "
            ))
        elif "SET_BY_CALLER" in calculation_text or "SET BY CALLER" in calculation_text:
            lines.extend(set_by_caller_report(
                get_prop(magnitude, "set_by_caller_magnitude"), "  "
            ))
        else:
            lines.append("  Unrecognized calculation type. Raw magnitude struct:")
            lines.append("    " + str(magnitude))

    lines.extend(tag_requirements_report(get_prop(modifier, "source_tags"), "Source Tag Requirements"))
    lines.extend(tag_requirements_report(get_prop(modifier, "target_tags"), "Target Tag Requirements"))
    lines.append("Evaluation Channel Settings: " + str(get_prop(modifier, "evaluation_channel_settings")))
    lines.append("")
    return lines


# -----------------------------------------------------------------------------
# Resolve the actual GameplayEffect object
# -----------------------------------------------------------------------------


def load_gameplay_effect_cdo():
    unreal.log("[GE Audit] Loading Blueprint asset: {}".format(ASSET_PATH))
    blueprint_asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blueprint_asset is None:
        raise RuntimeError("Could not load Blueprint asset: {}".format(ASSET_PATH))

    unreal.log("[GE Audit] Loaded asset object: {}".format(object_path(blueprint_asset)))
    unreal.log("[GE Audit] Loaded asset class: {}".format(object_path(blueprint_asset.get_class())))

    # Data-only GameplayEffect Blueprints store their editable GE properties on
    # the generated class default object (CDO), not on the UBlueprint asset.
    generated_class = None
    generated_class_method = getattr(blueprint_asset, "generated_class", None)
    if callable(generated_class_method):
        generated_class = generated_class_method()

    if generated_class is None:
        generated_class = get_prop(blueprint_asset, "generated_class")

    if generated_class is None:
        raise RuntimeError(
            "The loaded asset has no generated class. Confirm this path points "
            "to the Data-Only GameplayEffect Blueprint."
        )

    unreal.log("[GE Audit] Generated class: {}".format(object_path(generated_class)))
    cdo = unreal.get_default_object(generated_class)
    if cdo is None:
        raise RuntimeError("Could not retrieve the generated class default object (CDO).")

    unreal.log("[GE Audit] CDO: {}".format(object_path(cdo)))
    unreal.log("[GE Audit] CDO class: {}".format(object_path(cdo.get_class())))

    modifiers = get_prop(cdo, "modifiers", None)
    if modifiers is None:
        raise RuntimeError(
            "The CDO does not expose the 'modifiers' property. "
            "CDO class: {}".format(object_path(cdo.get_class()))
        )

    return blueprint_asset, generated_class, cdo, modifiers


# -----------------------------------------------------------------------------
# Report generation
# -----------------------------------------------------------------------------


def build_report(blueprint_asset, generated_class, gameplay_effect, modifiers):
    report = []
    duration_policy = get_prop(gameplay_effect, "duration_policy")
    duration_magnitude = get_prop(gameplay_effect, "duration_magnitude")
    max_duration_magnitude = get_prop(gameplay_effect, "max_duration_magnitude")
    period = get_prop(gameplay_effect, "period")
    stacking_type = get_prop(gameplay_effect, "stacking_type")
    execute_on_application = get_prop(gameplay_effect, "execute_periodic_effect_on_application")

    report.extend([
        "GAMEPLAY EFFECT CONFIGURATION REPORT",
        "Asset: " + object_path(blueprint_asset),
        "Generated Class: " + object_path(generated_class),
        "Class Default Object: " + object_path(gameplay_effect),
        "Asset Path: " + ASSET_PATH,
        "Generated: " + datetime.datetime.now().isoformat(sep=" ", timespec="seconds"),
        "Unreal Engine: 5.7",
        "Read Mode: Blueprint Generated Class Default Object (CDO)",
        "",
        "ASSET SUMMARY",
        "-" * 96,
        "Duration Policy: " + enum_text(duration_policy),
        "Duration Magnitude:",
    ])
    report.extend(gameplay_effect_magnitude_report(duration_magnitude, "  "))
    report.append("Max Duration Magnitude:")
    report.extend(gameplay_effect_magnitude_report(max_duration_magnitude, "  "))
    report.append("Period:")
    report.extend(scalable_float_report(period, "  "))
    report.append("Execute Periodic Effect on Application: " + str(execute_on_application))
    report.append("Stacking Type: " + enum_text(stacking_type))
    report.append("Modifier Count: {}".format(len(modifiers)))
    report.append("Expected Modifier Count: {}".format(EXPECTED_MODIFIER_COUNT))

    if len(modifiers) == EXPECTED_MODIFIER_COUNT:
        report.append("Modifier Count Check: PASS")
    else:
        report.append(
            "Modifier Count Check: WARNING - expected {}, found {}".format(
                EXPECTED_MODIFIER_COUNT, len(modifiers)
            )
        )

    report.extend(["", "MODIFIER DETAILS", ""])
    for index, modifier in enumerate(modifiers):
        report.extend(modifier_report(modifier, index))

    report.extend([
        "END OF REPORT",
        "",
        "Note: This report reads the generated class CDO only. No asset was modified or saved.",
    ])
    return report, len(modifiers)


def main():
    blueprint_asset, generated_class, cdo, modifiers = load_gameplay_effect_cdo()
    report_lines, modifier_count = build_report(
        blueprint_asset, generated_class, cdo, modifiers
    )

    output_dir = os.path.join(unreal.Paths.project_saved_dir(), REPORT_FOLDER_NAME)
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, REPORT_FILE_NAME)

    with open(output_path, "w", encoding="utf-8-sig", newline="\n") as report_file:
        report_file.write("\n".join(report_lines))

    if modifier_count != EXPECTED_MODIFIER_COUNT:
        unreal.log_error(
            "[GE Audit] Report written, but modifier count is {}, expected {}. "
            "Do not treat this export as complete.".format(
                modifier_count, EXPECTED_MODIFIER_COUNT
            )
        )
    else:
        unreal.log_warning(
            "[GE Audit] SUCCESS: Exported all {} modifiers to: {}".format(
                modifier_count, output_path
            )
        )

    unreal.log("[GE Audit] Read-only export complete. No Unreal asset was changed.")
    return output_path


try:
    REPORT_PATH = main()
except Exception as error:
    unreal.log_error("[GE Audit] EXPORT FAILED: {}".format(error))
    unreal.log_error(traceback.format_exc())
    raise
